#pragma once
#include <Native/BtiTexture.hpp>
#include <Native/GpuResources.hpp>
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <JSystem/JKernel/JKRArchive.hpp>

class NativeOwnedBti final:public JKRDisposer {
    Native::detail::BtiStorage texture;
public:
    NativeOwnedBti(const void* source,size_t size):texture(source,size){
        Native::gpuResources().add(texture.bytes.data(),texture.bytes.size());
        try{Native::markNativeBti(texture.texture(),true);}catch(...){Native::gpuResources().remove(texture.bytes.data());throw;}
    }
    ~NativeOwnedBti() override {Native::markNativeBti(texture.texture(),false);Native::gpuResources().remove(texture.bytes.data());}
    ResTIMG* header(){return const_cast<ResTIMG*>(texture.texture());}
};
inline ResTIMG* loadNativeArchiveBti(JKRArchive& archive,const void* source,JKRHeap* heap){
    const s32 size=source?archive.getResSize(source):0;
    if(size<32)throw std::invalid_argument("Missing or unbounded standalone BTI resource");
    // The disposer releases the GPU mapping and host storage at heap teardown.
    return (new(heap,0) NativeOwnedBti(source,size))->header();
}
