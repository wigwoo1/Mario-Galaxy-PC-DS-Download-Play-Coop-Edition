#pragma once

#include "Game/Util/Array.hpp"
#include <revolution/os/OSMessage.h>
#include <revolution/types.h>
#include <Native/ReadCompletion.hpp>

class JKRHeap;

class FileHolderFileEntry {
public:
    FileHolderFileEntry(const char*, JKRHeap*, void*);
    ~FileHolderFileEntry();
    static void operator delete(void*) noexcept;
    void waitReadDone();
    void setContext(void*, JKRHeap*);

    /* 0x00 */ s32 mEntryNum;
    /* 0x04 */ void* mContext;
    /* 0x08 */ JKRHeap* mHeap;
    /* 0x0C */ s32 mState;
    /* 0x10 */ OSMessageQueue mQueue;
    /* 0x30 */ OSMessage mMessage;
    /* 0x34 */ bool mContextSet;
    Native::ReadCompletion mNativeReadCompletion;
};

class FileHolder {
public:
    FileHolder();
    FileHolderFileEntry* add(const char*, JKRHeap*, void*);
    bool isExist(const char*) const;
    void* getContext(const char*) const;
    void removeIfIsEqualHeap(JKRHeap*);
    FileHolderFileEntry** removeFile(const char*);
    FileHolderFileEntry* findEntry(const char*) const;

    /* 0x0 */ MR::Vector< MR::AssignableArray< FileHolderFileEntry* > > mEntries;
};
