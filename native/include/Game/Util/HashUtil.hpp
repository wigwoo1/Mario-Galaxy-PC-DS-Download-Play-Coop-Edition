#pragma once
#include <revolution/types.h>
#include <cstdint>
class HashSortTable {
public:
    explicit HashSortTable(u32);
    ~HashSortTable();
    HashSortTable(const HashSortTable&)=delete;
    HashSortTable& operator=(const HashSortTable&)=delete;
    bool add(const char*,u32,bool);
    bool add(u32,u32);
    bool addOrSkip(u32,u32);
    void sort();
    bool search(u32,u32*);
    bool search(const char*,u32*);
    bool search(const char*,const char*,u32*);
    void swap(const char*,const char*);
    // Explicit pointer API avoids narrowing and writing through a u32 alias.
    bool addPointer(const char*,const void*,bool);
    bool searchPointer(const char*,void**);
    bool mHasBeenSorted=false;
    u32* mHashCodes=nullptr;
    std::uintptr_t* _8=nullptr;
    u16* _C=nullptr;
    u16* _10=nullptr;
    u32 mCurrentLength=0;
    u32 mMaxLength=0;
private:
    bool addValue(u32,std::uintptr_t,bool);
    bool searchValue(u32,std::uintptr_t*);
};
namespace MR { u32 getHashCode(const char*); u32 getHashCodeLower(const char*); }
