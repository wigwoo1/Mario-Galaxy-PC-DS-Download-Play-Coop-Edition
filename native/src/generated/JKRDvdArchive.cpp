#include <new>
#include "../native_dvd_port.hpp"
#include "../native_jkr_archive_port.hpp"
#include <cstdint>
#include <cstddef>
class JKRHeap; void* operator new(std::size_t, JKRHeap*, int);
#define ALIGN_PREV(X,N) ((X)&~(decltype(X))((N)-1))
#define ALIGN_NEXT(X,N) ALIGN_PREV((X)+(N)-1,N)
#include "JSystem/JKernel/JKRDvdArchive.hpp"
#include "JSystem/JKernel/JKRAramArchive.hpp"
#include "JSystem/JKernel/JKRArchive.hpp"
#include "JSystem/JKernel/JKRCompression.hpp"
#include "JSystem/JKernel/JKRDecomp.hpp"
#include "JSystem/JKernel/JKRDvdFile.hpp"
#include "JSystem/JKernel/JKRDvdRipper.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"
#include "JSystem/JUtility/JUTException.hpp"
#include <revolution/types.h>
#include <cstdlib>
#include <cstring>
#include <mem.h>

JKRDvdArchive::JKRDvdArchive(s32 entryNum, EMountDirection mountDir) : JKRArchive(entryNum, MOUNT_MODE_DVD) {
    mMountDir = mountDir;

    if (!open(entryNum)) {
        return;
    }

    mLoaderType = RARC_MAGIC;
    mLoaderName = mStringTable + mDirs->mNameOffset;

    prependVolumeList(&mLoaderLink);

    mIsMounted = true;
}

JKRDvdArchive::~JKRDvdArchive() {
    if (mIsMounted == true) {
        if (mInfoBlock != nullptr) {
            SDIFileEntry* entry = mFiles;
            for (int i = 0; i < mInfoBlock->mNrFiles; i++) {
                if (entry->mFileData != nullptr) {
                    JKRFreeToHeap(mHeap, entry->mFileData);
                }

                entry++;
            }

            nativeDetachRarcMetadata(this);
            mInfoBlock = nullptr;
        }

        if (mExpandSizes != nullptr) {
            JKRFree(mExpandSizes);
            mExpandSizes = nullptr;
        }

        if (mFile != nullptr) {
            delete mFile;
        }

        removeVolumeList(&mLoaderLink);
        mIsMounted = false;
    }
}

bool JKRDvdArchive::open(s32 entryNum) {
 mInfoBlock=nullptr;mDirs=nullptr;mFiles=nullptr;mStringTable=nullptr;mExpandSizes=nullptr;mFile=nullptr;_64=0;
 std::vector<uint8_t> bytes;
 if(!nativeReadDvdEntry(entryNum,bytes) || !nativeRegisterRarc(bytes.data(),bytes.size())){mMountMode=MOUNT_MODE_0;return false;}
 std::size_t payload=0;
 bool attached=nativeAttachRarcMetadata(this,bytes.data(),&payload);
 nativeUnregisterRarc(bytes.data());
 if(!attached){mMountMode=MOUNT_MODE_0;return false;}
 _5C=bytes.size()>=4 && !std::memcmp(bytes.data(),"Yaz0",4)?2:bytes.size()>=4 && !std::memcmp(bytes.data(),"Yay0",4)?1:0;
 _64=static_cast<u32>(payload);
 void* fileStorage=JKRHeap::alloc(sizeof(JKRDvdFile),32,JKRHeap::getSystemHeap());
 mFile=fileStorage?new (fileStorage) JKRDvdFile(entryNum):nullptr;
 if(!mFile || !mFile->isAvailable()){delete mFile;mFile=nullptr;nativeDetachRarcMetadata(this);mMountMode=MOUNT_MODE_0;return false;}
 bool compressed=false;
 for(u32 i=0;i<mInfoBlock->mNrFiles;++i)if(mFiles[i].mFlag&FILE_FLAG_FILE)compressed|=(mFiles[i].mFlag&FILE_FLAG_COMPRESSED)!=0;
 if(compressed){mExpandSizes=static_cast<s32*>(JKRHeap::alloc(mInfoBlock->mNrFiles*sizeof(s32),mMountDir==MOUNT_DIRECTION_1?32:-32,mHeap));if(!mExpandSizes){delete mFile;mFile=nullptr;nativeDetachRarcMetadata(this);mMountMode=MOUNT_MODE_0;return false;}std::memset(mExpandSizes,0,mInfoBlock->mNrFiles*sizeof(s32));}
 return true;
}



void* JKRDvdArchive::fetchResource(JKRArchive::SDIFileEntry* pArg1, u32* pSize) {
    u32 emergency_pointer;
    if (pSize == nullptr) {
        pSize = &emergency_pointer;
    }

    JKRCompression compression = JKRConvertAttrToCompressionType(pArg1->mFlag);

    if (pArg1->mFileData == nullptr) {
        u8* out;
        u32 size = fetchResource_subroutine(mEntryNum, _64 + pArg1->mDataOffset, pArg1->mDataSize, mHeap, compression, _5C, &out);
        *pSize = size;
        if (size == 0) {
            return nullptr;
        }

        pArg1->mFileData = out;
        if (compression == 2) {
            setExpandSize(pArg1, *pSize);
        }
    } else if (compression == 2) {
        u32 size = getExpandSize(pArg1);
        *pSize = size;
    } else {
        *pSize = pArg1->mDataSize;
    }

    return pArg1->mFileData;
}

void* JKRDvdArchive::fetchResource(void* pArg1, u32 arg2, JKRArchive::SDIFileEntry* pEntry, u32* pSize) {
    u32 fetchedSize = pEntry->mDataSize;
    JKRCompression compression = JKRConvertAttrToCompressionType(pEntry->mFlag);

    if (pEntry->mFileData == nullptr) {
        fetchedSize = fetchResource_subroutine(mEntryNum, _64 + pEntry->mDataOffset, pEntry->mDataSize, static_cast< u8* >(pArg1),
                                               ALIGN_PREV(arg2, 32), compression, _5C);
    } else {
        if (compression == 2) {
            u32 size = getExpandSize(pEntry);

            if (size != 0) {
                fetchedSize = size;
            }
        }

        if (fetchedSize > arg2) {
            fetchedSize = arg2;
        }

        JKRHeap::copyMemory(pArg1, pEntry->mFileData, fetchedSize);
    }

    if (pSize != nullptr) {
        *pSize = fetchedSize;
    }

    return pArg1;
}

u32 JKRDvdArchive::getExpandedResSize(const void* pArg) const {
    if (mExpandSizes == nullptr) {
        return getResSize(pArg);
    }

    SDIFileEntry* fileEntry = findPtrResource(pArg);
    if (fileEntry == nullptr) {
        return -1;
    }

    if ((fileEntry->mFlag & FILE_FLAG_COMPRESSED) == 0) {
        return getResSize(pArg);
    }

    u32 size = getExpandSize(fileEntry);
    if (size != 0) {
        return size;
    }

    u8 buff[0x40];
    u8* alignedPointer = reinterpret_cast< u8* >((ALIGN_NEXT(reinterpret_cast< uintptr_t >(buff), 32)));

    JKRDvdRipper::loadToMainRAM(mEntryNum, alignedPointer, EXPAND_SWITCH_UNKNOWN2, 0x20, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD,
                                _64 + fileEntry->mDataOffset, nullptr, nullptr);

    DCInvalidateRange(alignedPointer, 0x20);

    u32 size2 = JKRDecompExpandSize(alignedPointer);
    const_cast< JKRDvdArchive* >(this)->setExpandSize(fileEntry, size2);

    return size2;
}

u32 JKRDvdArchive::fetchResource_subroutine(s32 entryNum, u32 offset, u32 sourceSize, u8* pDestination, u32 destinationSize, int resourceCompression,
                                            int archiveCompression) {
    u32 readSize = ALIGN_NEXT(sourceSize, 32);
    u32 alignedDestinationSize = ALIGN_PREV(destinationSize, 32);

    switch (archiveCompression) {
    case 0: {
        switch (resourceCompression) {
        case 0: {
            if (readSize > alignedDestinationSize) {
                readSize = alignedDestinationSize;
            }

            JKRDvdRipper::loadToMainRAM(entryNum, pDestination, EXPAND_SWITCH_UNKNOWN0, readSize, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD,
                                        offset, nullptr, nullptr);
            DCInvalidateRange(pDestination, readSize);
            return readSize;
        }

        case 1:
        case 2: {
            u8 headerBuffer[0x40];
            u8* header = reinterpret_cast< u8* >((ALIGN_NEXT(reinterpret_cast< uintptr_t >(headerBuffer), 32)));

            JKRDvdRipper::loadToMainRAM(entryNum, header, EXPAND_SWITCH_UNKNOWN2, 0x20, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD, offset,
                                        nullptr, nullptr);
            DCInvalidateRange(header, 0x20);

            u32 readSize = JKRDecompExpandSize(header);
            u32 alignedExpandedSize = ALIGN_NEXT(readSize, 32);
            if (alignedExpandedSize > alignedDestinationSize) {
                alignedExpandedSize = alignedDestinationSize;
            }

            JKRDvdRipper::loadToMainRAM(entryNum, pDestination, EXPAND_SWITCH_UNKNOWN1, alignedExpandedSize, nullptr,
                                        JKRDvdRipper::ALLOC_DIRECTION_FORWARD, offset, nullptr, nullptr);
            DCInvalidateRange(pDestination, alignedExpandedSize);

            return readSize;
        }
        }
    }
    case 2: {
        if (sourceSize > alignedDestinationSize) {
            sourceSize = alignedDestinationSize;
        }

        JKRDvdRipper::loadToMainRAM(entryNum, pDestination, EXPAND_SWITCH_UNKNOWN1, sourceSize, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD,
                                    offset, nullptr, nullptr);
        DCInvalidateRange(pDestination, sourceSize);
        return sourceSize;
    }
    case 1: {
        const char* SZP_ERROR = "Sorry, not applied for SZP archive.\n";
        JUTException::panic(__FILE__, 0x289, SZP_ERROR);
    }

    default: {
        const char* SEQUENCE_ERROR = "??? bad sequence\n";
        JUTException::panic(__FILE__, 0x28D, SEQUENCE_ERROR);
    }
    }

    return 0;
}

u32 JKRDvdArchive::fetchResource_subroutine(s32 entryNum, u32 offset, u32 sourceSize, JKRHeap* pHeap, int resourceCompression, int archiveCompression,
                                            u8** pOutBuffer) {
    u32 alignedSourceSize = ALIGN_NEXT(sourceSize, 32);
    u8* buffer;
    u32 expandedSize;

    switch (archiveCompression) {
    case 0: {
        switch (resourceCompression) {
        case 0: {
            buffer = static_cast< u8* >(pHeap->alloc(alignedSourceSize, 0x20, pHeap));
            JKRDvdRipper::loadToMainRAM(entryNum, buffer, EXPAND_SWITCH_UNKNOWN0, alignedSourceSize, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD,
                                        offset, nullptr, nullptr);
            DCInvalidateRange(buffer, alignedSourceSize);

            *pOutBuffer = buffer;
            return alignedSourceSize;
        }

        case 1:
        case 2: {
            u8 headerBuffer[0x40];
            u8* header = reinterpret_cast< u8* >((ALIGN_NEXT(reinterpret_cast< uintptr_t >(headerBuffer), 32)));

            JKRDvdRipper::loadToMainRAM(entryNum, header, EXPAND_SWITCH_UNKNOWN2, 0x20, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD, offset,
                                        nullptr, nullptr);
            DCInvalidateRange(header, 0x20);

            expandedSize = JKRDecompExpandSize(header);
            buffer = static_cast< u8* >(pHeap->alloc(expandedSize, 0x20, pHeap));

            JKRDvdRipper::loadToMainRAM(entryNum, buffer, EXPAND_SWITCH_UNKNOWN1, expandedSize, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD,
                                        offset, nullptr, nullptr);
            DCInvalidateRange(buffer, expandedSize);
            *pOutBuffer = buffer;
            return expandedSize;
        }
        }
    }
    case 2: {
        buffer = static_cast< u8* >(pHeap->alloc(alignedSourceSize, 0x20, pHeap));
        JKRDvdRipper::loadToMainRAM(entryNum, buffer, EXPAND_SWITCH_UNKNOWN1, sourceSize, nullptr, JKRDvdRipper::ALLOC_DIRECTION_FORWARD, offset,
                                    nullptr, nullptr);
        DCInvalidateRange(buffer, sourceSize);
        *pOutBuffer = buffer;
        return alignedSourceSize;
    }
    case 1: {
        const char* SZP_ERROR_TYPO = "Sorry, not applied SZP archive.\n";
        JUTException::panic(__FILE__, 0x2F2, SZP_ERROR_TYPO);
    }

    default: {
        const char* SEQUENCE_ERROR = "??? bad sequence\n";
        JUTException::panic(__FILE__, 0x2F6, SEQUENCE_ERROR);
    }
    }

    return 0;
}
