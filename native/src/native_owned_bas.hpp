#pragma once
#include <Native/BasAnimation.hpp>
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRArchive.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>

class NativeOwnedBas final : public JKRDisposer {
    Native::BasAnimation storage;
public:
    NativeOwnedBas(const void* data,std::size_t size):storage(data,size){}
    JAUSoundAnimation* animation(){return storage.animation();}
};
inline JAUSoundAnimation* loadNativeArchiveBas(JKRArchive& archive,const void* data,JKRHeap* heap) {
    const s32 size=data?archive.getResSize(data):0;
    if(!heap||size<8)throw std::invalid_argument("Missing or unbounded BAS archive resource");
    return (new(heap,0) NativeOwnedBas(data,static_cast<std::size_t>(size)))->animation();
}
