#pragma once
#include "native_tev_reg_animation.hpp"
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>

class NativeOwnedTevReg final : public JKRDisposer {
    NativeTevRegAnimation storage;
public:
    NativeOwnedTevReg(const void* data,size_t size):storage(data,size){}
    J3DAnmBase* animation(){return &storage;}
};
inline J3DAnmBase* loadNativeArchiveTevReg(const JKRArchive& archive,const void* data,JKRHeap* heap){
    if(!data)return nullptr;
    const s32 size=archive.getResSize(data);
    assetCheck(heap && size>=40,"BRK resource not owned by archive or truncated");
    return (new(heap,0) NativeOwnedTevReg(data,size_t(size)))->animation();
}
