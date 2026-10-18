#pragma once

#include <revolution.h>
#include <JSystem/JKernel/JKRHeap.hpp>
#include <cstdlib>

class JKRArchive;

class JKRFileFinder {
public:
    const char* mName;
    s32 mDirIndex;
    u16 mFileID;
    u16 mFileFlag;

    JKRFileFinder();
    virtual ~JKRFileFinder() {}
    virtual bool findNextFile() = 0;

    bool mHasMoreFiles;
    bool mFileIsFolder;
    u8 _12[2];

    // Native archive iterators use the host allocator so their lifetime does
    // not depend on freeing a tiny temporary object through Wii heap metadata.
    static void operator delete(void* memory) noexcept { std::free(memory); }
    static void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
};

class JKRArcFinder : public JKRFileFinder {
public:
    JKRArcFinder(JKRArchive*, long, long);
    virtual ~JKRArcFinder();
    virtual bool findNextFile();

    static void operator delete(void* memory) noexcept { std::free(memory); }
    static void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

    JKRArchive* mArchive;
    s32 mFirstIndex;
    s32 mLastIndex;
    s32 mCurrentIndex;
};
