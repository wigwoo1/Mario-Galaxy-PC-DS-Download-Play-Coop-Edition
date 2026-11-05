#include "../native_jkr_archive_port.hpp"
#include "JSystem/JKernel/JKRMemArchive.hpp"
#include "JSystem/JKernel/JKRAram.hpp"
#include "JSystem/JKernel/JKRDecomp.hpp"
#include "JSystem/JKernel/JKRDvdRipper.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"
#include "JSystem/JUtility/JUTException.hpp"
#include <revolution.h>
#include <cstring>

JKRMemArchive::JKRMemArchive() {
}

JKRMemArchive::JKRMemArchive(long entryNum, EMountDirection mountDir) : JKRArchive(entryNum, MOUNT_MODE_MEM) {
    mIsMounted = false;
    mMountDir = mountDir;

    if (!open(entryNum, mountDir)) {
        return;
    }

    mLoaderType = RARC_MAGIC;
    mLoaderName = mStringTable + mDirs->mNameOffset;

    prependVolumeList(&mLoaderLink);

    mIsMounted = true;
}

JKRMemArchive::~JKRMemArchive() { nativeDetachRarc(this);
    if (mIsMounted == true) {
        if (_6C && mHeader != nullptr) {
            JKRHeap::free(mHeader, mHeap);
        }

        removeVolumeList(&mLoaderLink);
        mIsMounted = false;
    }
}

void JKRMemArchive::fixedInit(std::intptr_t entryNum) {
    mIsMounted = false;
    mMountMode = MOUNT_MODE_MEM;
    _34 = 1;
    _58 = 2;
    mHeap = JKRHeap::sCurrentHeap;
    mEntryNum = entryNum;

    if (sCurrentVolume != nullptr) {
        return;
    }

    sCurrentVolume = this;
    sCurrentDirID = 0;
}

bool JKRMemArchive::mountFixed(void* a1, JKRMemBreakFlag breakFlag) {
    if (check_mount_already(reinterpret_cast<std::intptr_t>(a1)) != nullptr) {
        return false;
    }

    fixedInit(reinterpret_cast<std::intptr_t>(a1));

    if (!open(a1, 0xFFFF, breakFlag)) {
        return false;
    }

    SDIDirEntry* firstDir = mDirs;
    char* stringTable = mStringTable;

    mLoaderType = RARC_MAGIC;
    mLoaderName = stringTable + firstDir->mNameOffset;

    prependVolumeList(&mLoaderLink);

    mIsMounted = true;
    _6C = breakFlag == JKR_MEM_BREAK_FLAG_1;

    return true;
}

bool JKRMemArchive::open(long entryNum, EMountDirection mountDir) {
    mHeader = nullptr;
    mInfoBlock = nullptr;
    mFileDataStart = nullptr;
    mDirs = nullptr;
    mFiles = nullptr;
    mStringTable = nullptr;
    _6C = false;
    mMountDir = mountDir;
    u32 expandedSize = 0;
    const auto direction = mountDir == MOUNT_DIRECTION_1
        ? JKRDvdRipper::ALLOC_DIRECTION_FORWARD : JKRDvdRipper::ALLOC_DIRECTION_BACKWARD;
    void* data = JKRDvdRipper::loadToMainRAM(entryNum, nullptr, EXPAND_SWITCH_UNKNOWN1,
        0, mHeap, direction, 0, reinterpret_cast<int*>(&_5C), &expandedSize);
    if (!data) { mMountMode = MOUNT_MODE_0; return false; }
    DCInvalidateRange(data, expandedSize);
    const bool registered = nativeRegisterRarc(data, expandedSize);
    const bool attached = registered && nativeAttachRarc(this, data);
    if (registered) nativeUnregisterRarc(data);
    JKRHeap::free(data, mHeap);
    if (!attached) mMountMode = MOUNT_MODE_0;
    return attached;
}

bool JKRMemArchive::open(void* pData, u32 a2, JKRMemBreakFlag breakFlag) {
    if (!nativeAttachRarc(this, pData)) return false;
    mHeap = JKRHeap::findFromRoot(pData);
    _5C = 0;
    return true;
}

void* JKRMemArchive::fetchResource(SDIFileEntry* pFile, u32* pSize) {
    if (pFile->mFileData == nullptr) {
        pFile->mFileData = mFileDataStart + pFile->mDataOffset;
    }

    if (pSize != nullptr) {
        *pSize = pFile->mDataSize;
    }

    return pFile->mFileData;
}

void* JKRMemArchive::fetchResource(void* pData, u32 dataSize, SDIFileEntry* pFile, u32* pSize) {
    u32 size = pFile->mDataSize;

    if (size > dataSize) {
        size = dataSize;
    }

    if (pFile->mFileData != nullptr) {
        memcpy(pData, pFile->mFileData, size);
    } else {
        s32 compression;

        if ((pFile->mFlag & FILE_FLAG_COMPRESSED) == 0) {
            compression = JKR_COMPRESSION_NONE;
        } else if ((pFile->mFlag & FILE_FLAG_IS_YAZ0) != 0) {
            compression = JKR_COMPRESSION_SZS;
        } else {
            compression = JKR_COMPRESSION_SZP;
        }

        size = fetchResource_subroutine(mFileDataStart + pFile->mDataOffset, size, reinterpret_cast< u8* >(pData), dataSize, compression);
    }

    if (pSize != nullptr) {
        *pSize = size;
    }

    return pData;
}

void JKRMemArchive::removeResourceAll() {
    if (mInfoBlock == nullptr) {
        return;
    }

    if (mMountMode == MOUNT_MODE_MEM) {
        return;
    }

    SDIFileEntry* current = mFiles;

    for (s32 i = 0; i < mInfoBlock->mNrFiles; i++) {
        if (current->mFileData != nullptr) {
            current->mFileData = nullptr;
        }
    }
}

bool JKRMemArchive::removeResource(void* pResource) {
    SDIFileEntry* file = findPtrResource(pResource);

    if (file == nullptr) {
        return false;
    }

    file->mFileData = nullptr;
    return true;
}

s32 JKRMemArchive::fetchResource_subroutine(unsigned char* pSrc, u32 srcSize, unsigned char* pDst, u32 dstSize, int compression) {
    switch (compression) {
    case JKR_COMPRESSION_NONE:
        if (srcSize > dstSize) {
            srcSize = dstSize;
        }

        memcpy(pDst, pSrc, srcSize);

        return srcSize;
    case JKR_COMPRESSION_SZP:
    case JKR_COMPRESSION_SZS: {
        u32 size = JKRDecompExpandSize(pSrc);

        if (size > dstSize) {
            size = dstSize;
        }

        JKRDecomp::orderSync(pSrc, pDst, size, 0);
        return size;
    }
    default:
        JUTException::panic(__FILE__, 723, "??? bad sequence\n");
        break;
    }

    return 0;
}

u32 JKRMemArchive::getExpandedResSize(const void* pResource) const {
    SDIFileEntry* file = findPtrResource(pResource);

    if (file == nullptr) {
        return -1;
    }

    if ((file->mFlag & FILE_FLAG_COMPRESSED) == 0) {
        return getResSize(pResource);
    }

    return JKRDecompExpandSize(reinterpret_cast< u8* >(const_cast< void* >(pResource)));
}
