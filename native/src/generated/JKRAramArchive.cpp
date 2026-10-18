#include "../native_dvd_port.hpp"
#include "../native_jkr_archive_port.hpp"
#include "../native_jkr_aram_port.hpp"
#include "../native_rarc_tables.hpp"
#include <JSystem/JKernel/JKRDecomp.hpp>
#include <new>
#define ALIGN_PREV(X,N) ((X)&~(decltype(X))((N)-1))
#define ALIGN_NEXT(X,N) ALIGN_PREV((X)+(N)-1,N)
#include "JSystem/JKernel/JKRDvdFile.hpp"
#include "JSystem/JKernel/JKRAramArchive.hpp"
#include "JSystem/JKernel/JKRAram.hpp"
#include "JSystem/JKernel/JKRCompression.hpp"
#include "JSystem/JKernel/JKRDvdAramRipper.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"
#include "JSystem/JUtility/JUTException.hpp"
#include <mem.h>

extern "C" int abs(int);

JKRAramArchive::JKRAramArchive(s32 entryNum, EMountDirection mountDir) : JKRArchive(entryNum, MOUNT_MODE_ARAM) {
    mMountDir = mountDir;

    if (!open(entryNum)) {
        return;
    }

    mLoaderType = RARC_MAGIC;
    mLoaderName = mStringTable + mDirs->mNameOffset;

    prependVolumeList(&mLoaderLink);

    mIsMounted = true;
}

JKRAramArchive::~JKRAramArchive() {
    if (mIsMounted == true) {
        if (mInfoBlock != NULL) {
            SDIFileEntry* entry = mFiles;
            for (int i = 0; i < mInfoBlock->mNrFiles; i++) {
                if (entry->mFileData != NULL) {
                    JKRFreeToHeap(mHeap, entry->mFileData);
                }
                entry++;
            }

            nativeDetachRarcMetadata(this);
            mInfoBlock = NULL;
        }

        if (mExpandSizes != NULL) {
            JKRFree(mExpandSizes);
            mExpandSizes = NULL;
        }

        if (mDvdFile != NULL) {
            delete mDvdFile;
        }

        if (mBlock != NULL) {
            delete mBlock;
        }

        removeVolumeList(&mLoaderLink);
        mIsMounted = false;
    }
}

bool JKRAramArchive::open(s32 entryNum) {
 mInfoBlock=nullptr;mDirs=nullptr;mFiles=nullptr;mStringTable=nullptr;mBlock=nullptr;mDvdFile=nullptr;mExpandSizes=nullptr;
 auto fail=[&]{delete mBlock;mBlock=nullptr;delete mDvdFile;mDvdFile=nullptr;if(mExpandSizes){JKRHeap::free(mExpandSizes,mHeap);mExpandSizes=nullptr;}nativeDetachRarcMetadata(this);mMountMode=MOUNT_MODE_0;return false;};
 std::vector<uint8_t> bytes;
 if(!nativeReadDvdEntry(entryNum,bytes) || !nativeRegisterRarc(bytes.data(),bytes.size()))return fail();
 bool attached=nativeAttachRarcMetadata(this,bytes.data());nativeUnregisterRarc(bytes.data());if(!attached)return fail();
 auto tables=nativeOwnedRarcTables(this);if(!tables || !JKRAram::sAramObject)return fail();
 _5C=bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4)?2:bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4)?1:0;
 size_t size=tables->header.mTotalDataSize;if(!size || size>0xffffffe0u)return fail();
 size_t rounded=(size+31)&~size_t(31);
 mBlock=JKRAram::sAramObject->mAramHeap->alloc(u32(rounded),mMountDir==MOUNT_DIRECTION_1?JKRAramHeap::HEAD:JKRAramHeap::TAIL);
 if(!mBlock)return fail();
 std::vector<uint8_t> payload(rounded);std::memcpy(payload.data(),tables->bytes.data()+tables->payload,size);
 if(!nativeUploadAram(mBlock->getAddress(),payload.data(),payload.size()))return fail();
 void* storage=JKRHeap::alloc(sizeof(JKRDvdFile),32,JKRHeap::getSystemHeap());
 mDvdFile=storage?new(storage) JKRDvdFile(entryNum):nullptr;if(!mDvdFile || !mDvdFile->isAvailable())return fail();
 bool compressed=false;for(const auto& file:tables->files)if(file.mFlag&FILE_FLAG_FILE)compressed|=(file.mFlag&FILE_FLAG_COMPRESSED)!=0;
 if(compressed){mExpandSizes=static_cast<s32*>(JKRHeap::alloc(mInfoBlock->mNrFiles*sizeof(s32),32,mHeap));if(!mExpandSizes)return fail();std::memset(mExpandSizes,0,mInfoBlock->mNrFiles*sizeof(s32));}
 return true;
}


void* JKRAramArchive::fetchResource(SDIFileEntry* pEntry, u32* pOutSize) {
    u32 outSize;
    u8* outBuf;
    if (pOutSize == NULL) {
        pOutSize = &outSize;
    }

    JKRCompression compression = JKRConvertAttrToCompressionType(static_cast< u8 >(pEntry->mFlag));
    if (pEntry->mFileData == NULL) {
        u32 size =
            JKRAramArchive::fetchResource_subroutine(pEntry->mDataOffset + mBlock->getAddress(), pEntry->mDataSize, mHeap, compression, &outBuf);

        *pOutSize = size;
        if (size == 0) {
            return NULL;
        }

        pEntry->mFileData = outBuf;
        if (compression == COMPRESSION_YAZ0) {
            this->setExpandSize(pEntry, *pOutSize);
        }
    } else {
        if (compression == COMPRESSION_YAZ0) {
            *pOutSize = this->getExpandSize(pEntry);
        } else {
            *pOutSize = pEntry->mDataSize;
        }
    }

    return pEntry->mFileData;
}

void* JKRAramArchive::fetchResource(void* buffer, u32 bufferSize, SDIFileEntry* pEntry, u32* resourceSize) {
    u32 size = pEntry->mDataSize;
    if (size > bufferSize) {
        size = bufferSize;
    }

    JKRCompression compression = JKRConvertAttrToCompressionType(static_cast< u8 >(pEntry->mFlag));
    if (pEntry->mFileData == NULL) {
        bufferSize = static_cast< s32 >(ALIGN_PREV(bufferSize, 0x20));
        size = JKRAramArchive::fetchResource_subroutine(pEntry->mDataOffset + mBlock->getAddress(), size, static_cast< u8* >(buffer), bufferSize,
                                                        compression);
    } else {
        if (compression == COMPRESSION_YAZ0) {
            u32 expandSize = this->getExpandSize(pEntry);
            if (expandSize != 0) {
                size = expandSize;
            }
        }
        if (size > bufferSize) {
            size = bufferSize;
        }
        JKRHeap::copyMemory(buffer, pEntry->mFileData, size);
    }

    if (resourceSize != NULL) {
        *resourceSize = size;
    }
    return buffer;
}

u32 JKRAramArchive::fetchResource_subroutine(u32 srcAram, u32 srcLength, u8* dst, u32 dstLength, int compression) {
    u32 outLen;
    u32 srcSize = ALIGN_NEXT(srcLength, 0x20);
    u32 dstSize = ALIGN_PREV(dstLength, 0x20);

    switch (compression) {
    case COMPRESSION_NONE:
        if (srcSize > dstSize) {
            srcSize = dstSize;
        }
        JKRAramToMainRam(srcAram, dst, srcSize, EXPAND_SWITCH_UNKNOWN0, dstSize, NULL, -1, &outLen);
        return outLen;
    case COMPRESSION_YAY0:
    case COMPRESSION_YAZ0:
        JKRAramToMainRam(srcAram, dst, srcSize, EXPAND_SWITCH_UNKNOWN1, dstSize, NULL, -1, &outLen);
        return outLen;
    default:
        JUTException::panic(__FILE__, 655, "??? bad sequence\n");
        return 0;
    }
}

u32 JKRAramArchive::fetchResource_subroutine(u32 entryNum, u32 length, JKRHeap* pHeap, int compression, u8** out) {
    u32 alignedLen = ALIGN_NEXT(length, 0x20);

    u8* buffer;
    switch (compression) {
    case COMPRESSION_NONE: {
        buffer = static_cast< u8* >(JKRAllocFromHeap(pHeap, alignedLen, 0x20));
        JKRAramToMainRam(entryNum, buffer, alignedLen, EXPAND_SWITCH_UNKNOWN0, alignedLen, NULL, -1, NULL);
        *out = buffer;
        return length;
    }
    case COMPRESSION_YAY0:
    case COMPRESSION_YAZ0: {
        u8 headerBuf[0x40];
        u8* alignHeader = reinterpret_cast< u8* >(ALIGN_NEXT(reinterpret_cast< std::uintptr_t >(&headerBuf[0]), sizeof(RarcHeader)));
        JKRAramToMainRam(entryNum, alignHeader, sizeof(RarcHeader), EXPAND_SWITCH_UNKNOWN0, 0, NULL, -1, NULL);
        u32 decompressedLen = ALIGN_NEXT(JKRDecompExpandSize(alignHeader), sizeof(RarcHeader));
        buffer = static_cast< u8* >(JKRAllocFromHeap(pHeap, decompressedLen, sizeof(RarcHeader)));
        u32 readLen;
        JKRAramToMainRam(entryNum, buffer, alignedLen, EXPAND_SWITCH_UNKNOWN1, decompressedLen, pHeap, -1, &readLen);
        *out = buffer;
        return readLen;
    }
    default:
        JUTException::panic(__FILE__, 713, "??? bad sequence\n");
        return 0;
    }
}

u32 JKRAramArchive::getExpandedResSize(const void* ptr) const {
    if (mExpandSizes == NULL) {
        return this->getResSize(ptr);
    }

    JKRArchive::SDIFileEntry* entry = this->findPtrResource(ptr);
    if (entry == NULL) {
        return 0xFFFFFFFF;
    }

    u8 flags = entry->mFlag;
    if ((flags & 4) == 0) {
        return this->getResSize(ptr);
    }

    u32 expandSize = this->getExpandSize(entry);
    if (expandSize != 0) {
        return expandSize;
    }

    u8 tmpBuf[0x40];
    u8* buf = reinterpret_cast< u8* >(ALIGN_PREV(reinterpret_cast< std::uintptr_t >(&tmpBuf[0x1F]), 0x20));
    JKRAramToMainRam(entry->mDataOffset + mBlock->getAddress(), buf, 0x20, EXPAND_SWITCH_UNKNOWN0, 0, NULL, -1, NULL);
    u32 expandSize2 = JKRDecompExpandSize(buf);
    const_cast< JKRAramArchive* >(this)->setExpandSize(entry, expandSize2);
    return expandSize2;
}
