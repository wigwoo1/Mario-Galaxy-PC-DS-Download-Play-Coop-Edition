#include "../native_dvd_port.hpp"
#include "../native_jkr_archive_port.hpp"
#include "../native_jkr_aram_port.hpp"
#include "../native_rarc_tables.hpp"
#include <new>
#include <Native/HeapPlacement.hpp>
#define ALIGN_NEXT(X,N) (((X)+(N)-1)&~(decltype(X))((N)-1))
#include "JSystem/JKernel/JKRCompArchive.hpp"
#include "JSystem/JKernel/JKRAram.hpp"
#include "JSystem/JKernel/JKRAramArchive.hpp"
#include "JSystem/JKernel/JKRAramBlock.hpp"
#include "JSystem/JKernel/JKRDecomp.hpp"
#include "JSystem/JKernel/JKRDvdAramRipper.hpp"
#include "JSystem/JKernel/JKRDvdArchive.hpp"
#include "JSystem/JKernel/JKRDvdFile.hpp"
#include "JSystem/JKernel/JKRDvdRipper.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"
#include "JSystem/JKernel/JKRMemArchive.hpp"
#include "JSystem/JUtility/JUTException.hpp"

#include <mem.h>
#include <stdint.h>

extern "C" int abs(int);

JKRCompArchive::JKRCompArchive(s32 entryNum, JKRArchive::EMountDirection eMountDirection) : JKRArchive(entryNum, MOUNT_MODE_COMP) {
    mMountDir = eMountDirection;

    if (!open(entryNum)) {
        return;
    }

    mLoaderType = 'RARC';
    mLoaderName = &mStringTable[mDirs->mNameOffset];
    /* SMG1 change */
    prependVolumeList(&mLoaderLink);
    // sVolumeList.prepend(&mLoaderLink);
    mIsMounted = true;
}

JKRCompArchive::~JKRCompArchive() {
    if (mInfoBlock != NULL) {
        SDIFileEntry* file = mFiles;
        for (int i = 0; i < mInfoBlock->mNrFiles; i++) {
            if ((file->mFlag & FILE_FLAG_MRAM) == 0 && file->mFileData != NULL) {
                JKRFreeToHeap(mHeap, file->mFileData);
            }

            file++;
        }

        nativeDetachRarcMetadata(this);
        mInfoBlock = NULL;
    }

    if (mAramPart != NULL) {
        delete mAramPart;
    }

    if (mExpandSizes != NULL) {
        JKRFree(mExpandSizes);
        mExpandSizes = NULL;
    }

    if (mDvdFile != NULL) {
        delete mDvdFile;
    }

    removeVolumeList(&mLoaderLink);
    mIsMounted = false;
}

bool JKRCompArchive::open(s32 entryNum) {
 mInfoBlock=nullptr;mDirs=nullptr;mFiles=nullptr;mStringTable=nullptr;mExpandSizes=nullptr;
 field_0x64=0;mAramPart=nullptr;field_0x6c=0;mDvdFile=nullptr;
 mSizeOfMemPart=0;mSizeOfAramPart=0;field_0x7c=0;
 auto fail=[&]{delete mAramPart;mAramPart=nullptr;delete mDvdFile;mDvdFile=nullptr;if(mExpandSizes){JKRHeap::free(mExpandSizes,mHeap);mExpandSizes=nullptr;}nativeDetachRarcMetadata(this);field_0x64=0;mMountMode=MOUNT_MODE_0;return false;};
 std::vector<uint8_t> bytes;
 if(!nativeReadDvdEntry(entryNum,bytes) || !nativeRegisterRarc(bytes.data(),bytes.size()))return fail();
 bool attached=nativeAttachRarcMetadata(this,bytes.data());nativeUnregisterRarc(bytes.data());
 if(!attached)return fail();
 auto tables=nativeOwnedRarcTables(this);if(!tables)return fail();
 try{for(const auto& file:tables->files)if(file.mFlag&FILE_FLAG_FILE)tables->filePartition(file);}catch(const std::exception&){return fail();}
 _5C=bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4)?2:bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4)?1:0;
 auto main=tables->mainRamPartition(),aram=tables->aramPartition();
 mSizeOfMemPart=u32(main.size);mSizeOfAramPart=u32(aram.size);
 field_0x64=reinterpret_cast<std::uintptr_t>(tables->bytes.data()+main.offset);
 field_0x6c=int(tables->payload);
 void* storage=JKRHeap::alloc(sizeof(JKRDvdFile),32,JKRHeap::getSystemHeap());
 mDvdFile=storage?new(storage) JKRDvdFile(entryNum):nullptr;
 if(!mDvdFile || !mDvdFile->isAvailable())return fail();
 if(aram.size){
  if(!JKRAram::sAramObject || aram.size>0xffffffe0u)return fail();
  size_t rounded=(aram.size+31)&~size_t(31);
  mAramPart=JKRAram::sAramObject->mAramHeap->alloc(u32(rounded),JKRAramHeap::HEAD);
  if(!mAramPart)return fail();
  std::vector<uint8_t> padded(rounded);std::memcpy(padded.data(),tables->bytes.data()+aram.offset,aram.size);
  if(!nativeUploadAram(mAramPart->getAddress(),padded.data(),padded.size()))return fail();
 }
 bool compressed=false;for(const auto& file:tables->files)if((file.mFlag&FILE_FLAG_FILE) && !(file.mFlag&FILE_FLAG_MRAM))compressed|=(file.mFlag&FILE_FLAG_COMPRESSED)!=0;
 if(compressed){mExpandSizes=static_cast<s32*>(JKRHeap::alloc(mInfoBlock->mNrFiles*sizeof(s32),32,mHeap));if(!mExpandSizes)return fail();std::memset(mExpandSizes,0,mInfoBlock->mNrFiles*sizeof(s32));}
 return true;
}


void* JKRCompArchive::fetchResource(SDIFileEntry* fileEntry, u32* pSize) {
    u32 ptrSize;
    u32 size = fileEntry->mDataSize;
    int compression = JKRConvertAttrToCompressionType(u8(fileEntry->mFlag));

    if (pSize == NULL) {
        pSize = &ptrSize;  // this makes barely any sense but ok
    }

    if (fileEntry->mFileData == NULL) {
        u32 flag = fileEntry->mFlag;
        if (flag & 0x10) {
            fileEntry->mFileData = (void*)(field_0x64 + fileEntry->mDataOffset);
            *pSize = size;
        } else if (flag & 0x20) {
            u8* data;
            size = JKRAramArchive::fetchResource_subroutine(fileEntry->mDataOffset + mAramPart->getAddress() - mSizeOfMemPart, size, mHeap,
                                                            compression, &data);
            *pSize = size;
            fileEntry->mFileData = data;
            if (compression == COMPRESSION_YAZ0) {
                setExpandSize(fileEntry, *pSize);
            }
        } else if (flag & 0x40) {
            u8* data;
            u32 resSize = JKRDvdArchive::fetchResource_subroutine(mEntryNum, field_0x6c + fileEntry->mDataOffset, fileEntry->mDataSize, mHeap,
                                                                  compression, _5C, &data);
            if (pSize != NULL) {
                *pSize = resSize;
            }
            fileEntry->mFileData = data;
            if (compression == COMPRESSION_YAZ0) {
                setExpandSize(fileEntry, *pSize);
            }
        }
    } else {
        if (pSize != NULL) {
            *pSize = fileEntry->mDataSize;
        }
    }
    return fileEntry->mFileData;
}

void* JKRCompArchive::fetchResource(void* data, u32 compressedSize, SDIFileEntry* fileEntry, u32* pSize) {
    u32 size = 0;
    u32 fileSize = fileEntry->mDataSize;
    u32 alignedSize = ALIGN_NEXT(fileSize, 32);
    u32 fileFlag = fileEntry->mFlag;
    int compression = JKRConvertAttrToCompressionType(u8(fileFlag));

    if (fileEntry->mFileData != NULL) {
        if (compression == COMPRESSION_YAZ0) {
            u32 expandSize = getExpandSize(fileEntry);
            if (expandSize != 0) {
                fileSize = expandSize;
            }
        }

        if (fileSize > compressedSize) {
            fileSize = compressedSize;
        }

        JKRHeap::copyMemory(data, fileEntry->mFileData, fileSize);
        size = fileSize;
    } else {
        if (fileFlag & 0x10) {
            size = JKRMemArchive::fetchResource_subroutine((u8*)(field_0x64 + fileEntry->mDataOffset), alignedSize, (u8*)data, compressedSize & ~31,
                                                           compression);
        } else if (fileFlag & 0x20) {
            size = JKRAramArchive::fetchResource_subroutine(fileEntry->mDataOffset + mAramPart->getAddress() - mSizeOfMemPart, alignedSize, (u8*)data,
                                                            compressedSize & ~31, compression);
        } else if (fileFlag & 0x40) {
            size = JKRDvdArchive::fetchResource_subroutine(mEntryNum, field_0x6c + fileEntry->mDataOffset, alignedSize, (u8*)data,
                                                           compressedSize & ~31, compression, _5C);
        } else {
            JUTException::panic(__FILE__, 776, "illegal archive.");
        }
    }

    if (pSize != NULL) {
        *pSize = size;
    }
    return data;
}

void JKRCompArchive::removeResourceAll() {
    if (mInfoBlock != NULL && mMountMode != 1) {
        SDIFileEntry* fileEntry = mFiles;
        for (int i = 0; i < mInfoBlock->mNrFiles; i++) {
            int tmp = fileEntry->mFlag;

            if (fileEntry->mFileData != NULL) {
                if (!(tmp & 0x10)) {
                    JKRFreeToHeap(mHeap, fileEntry->mFileData);
                }

                fileEntry->mFileData = NULL;
            }
        }
        fileEntry++;
    }
}

bool JKRCompArchive::removeResource(void* resource) {
    SDIFileEntry* fileEntry = findPtrResource(resource);
    if (!fileEntry)
        return false;

    u32 flags = fileEntry->mFlag;
    if ((flags & 0x10) == 0) {
        JKRFreeToHeap(mHeap, resource);
    }

    fileEntry->mFileData = NULL;
    return true;
}

u32 JKRCompArchive::getExpandedResSize(const void* resource) const {
    if (mExpandSizes == NULL) {
        return getResSize(resource);
    }

    SDIFileEntry* fileEntry = findPtrResource(resource);
    if (!fileEntry) {
        return 0xffffffff;
    }

    u8 flags = fileEntry->mFlag;
    if ((flags & 4) == 0) {  // not compressed
        return getResSize(resource);
    }

    if ((flags & 0x10) != 0) {
        u32 expandSize = JKRDecompExpandSize((u8*)resource);
        return expandSize;
    }

    u8 buf[64];
    u8* bufPtr = (u8*)ALIGN_NEXT((uintptr_t)buf, 32);
    if ((flags & 0x20) != 0) {
        JKRAramToMainRam(fileEntry->mDataOffset + mAramPart->getAddress(), bufPtr, sizeof(buf) / 2, EXPAND_SWITCH_UNKNOWN0, 0, NULL, -1, NULL);
        DCInvalidateRange(bufPtr, sizeof(buf) / 2);
    } else if ((flags & 0x40) != 0) {
        JKRDvdToMainRam(mEntryNum, bufPtr, EXPAND_SWITCH_UNKNOWN2, sizeof(buf) / 2, NULL, JKRDvdRipper::ALLOC_DIRECTION_FORWARD,
                        field_0x6c + fileEntry->mDataOffset, NULL, NULL);
        DCInvalidateRange(bufPtr, sizeof(buf) / 2);
    } else {
        JUTException::panic(__FILE__, 943, "illegal resource.");
    }
    u32 expandSize = JKRDecompExpandSize(bufPtr);
    const_cast< JKRCompArchive* >(this)->setExpandSize(fileEntry, expandSize);
    return expandSize;
}
