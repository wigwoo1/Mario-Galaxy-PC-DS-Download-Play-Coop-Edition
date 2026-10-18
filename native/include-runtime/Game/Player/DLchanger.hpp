#pragma once
#include <JSystem/JKernel/JKRHeap.hpp>
#include <revolution/types.h>
#include <new>
#include <stdexcept>
class J3DModelX;
struct DLholder { u8* mDL; u16 mSize; u16 _6; };
class DLchanger {
public:
    DLchanger(u8 count,u32 size):mBuffers(nullptr),mBufferCount(count),mCurrentBuffer(0) {
        if(count==0)throw std::invalid_argument("DLchanger needs at least one buffer");
        mBuffers=new DLholder[count]{};
        try { for(u32 i=0;i<count;++i)mBuffers[i].mDL=new (std::align_val_t{32}) u8[size]; }
        catch(...) { release();throw; }
    }
    ~DLchanger(){release();}
    DLchanger(const DLchanger&)=delete;
    DLchanger& operator=(const DLchanger&)=delete;
    void addDL(J3DModelX*);
    DLholder* swap(){mCurrentBuffer=(mCurrentBuffer+1)%mBufferCount;return &mBuffers[mCurrentBuffer];}
    DLholder* mBuffers;
    u8 mBufferCount;
    u8 mCurrentBuffer;
private:
    void release() noexcept {if(!mBuffers)return;for(u32 i=0;i<mBufferCount;++i)if(mBuffers[i].mDL)::operator delete[](mBuffers[i].mDL,std::align_val_t{32});delete[] mBuffers;mBuffers=nullptr;}
};
