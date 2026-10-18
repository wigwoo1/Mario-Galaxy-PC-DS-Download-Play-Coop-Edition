#pragma once
#include "graphics.hpp"
#include <Native/GpuResources.hpp>
#include <Native/HeapGpuRange.hpp>
#include <JSystem/J3DGraphAnimator/J3DMtxBuffer.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
// Own original matrix allocations; model data and its resource must outlive this owner.
struct NativeMatrixBufferResource {
    J3DMtxBuffer buffer;
    JKRHeap* heap;
    u32 views;
    unsigned bumpMaterials=0;
    size_t drawCount=0;std::vector<void*> gpuMatrices;
    explicit NativeMatrixBufferResource(J3DModelData& model,u32 viewCount=1):heap(JKRHeap::sCurrentHeap),views(viewCount){
        assetCheck(views>0 && views<=16,"Invalid model matrix view count");
        if(model.getModelDataType()==0)for(u16 i=0;i<model.getMaterialNum();++i){auto* scale=model.getMaterialNodePointer(i)->getNBTScale();assetCheck(scale!=nullptr,"Missing bump scale metadata");if(scale->mbHasScale)++bumpMaterials;}
        try{assetCheck(buffer.create(&model,views)==0,"Original matrix allocation failed");drawCount=model.getDrawMtxNum();
            if(drawCount)for(unsigned side=0;side<2;++side)for(u32 view=0;view<views;++view){
                if(buffer.mpDrawMtxArr[side]!=&J3DMtxBuffer::sNoUseDrawMtxPtr){auto* data=buffer.mpDrawMtxArr[side][view];gpuMatrices.push_back(data);if(!Native::hasHeapGpuRange(data))Native::gpuResources().addNativeWords(data,drawCount*sizeof(Mtx));}
                if(buffer.mpNrmMtxArr[side]!=&J3DMtxBuffer::sNoUseNrmMtxPtr){auto* data=buffer.mpNrmMtxArr[side][view];gpuMatrices.push_back(data);if(!Native::hasHeapGpuRange(data))Native::gpuResources().addNativeWords(data,drawCount*sizeof(Mtx33));}
            }}catch(...){release();throw;}
    }
    void refreshGpuMatrices(){if(!drawCount)return;for(unsigned side=0;side<2;++side)for(u32 view=0;view<views;++view){
        if(buffer.mpDrawMtxArr[side]!=&J3DMtxBuffer::sNoUseDrawMtxPtr)Native::gpuResources().refresh(buffer.mpDrawMtxArr[side][view],drawCount*sizeof(Mtx));
        if(buffer.mpNrmMtxArr[side]!=&J3DMtxBuffer::sNoUseNrmMtxPtr)Native::gpuResources().refresh(buffer.mpNrmMtxArr[side][view],drawCount*sizeof(Mtx33));}}
    void release() noexcept {
        for(void* data:gpuMatrices)if(!Native::releaseHeapGpuRange(data))Native::gpuResources().remove(data);gpuMatrices.clear();
        for(unsigned side=0;side<2;++side){
            if(buffer.mpBumpMtxArr[side]){for(unsigned material=0;material<bumpMaterials;++material){auto* array=buffer.mpBumpMtxArr[side][material];if(array){for(u32 view=0;view<views;++view)if(array[view]){Native::releaseHeapGpuRange(array[view]);JKRHeap::free(array[view],heap);}delete[] array;}}delete[] buffer.mpBumpMtxArr[side];}
            if(buffer.mpDrawMtxArr[side] && buffer.mpDrawMtxArr[side]!=&J3DMtxBuffer::sNoUseDrawMtxPtr){for(u32 view=0;view<views;++view)if(buffer.mpDrawMtxArr[side][view])JKRHeap::free(buffer.mpDrawMtxArr[side][view],heap);delete[] buffer.mpDrawMtxArr[side];}
            if(buffer.mpNrmMtxArr[side] && buffer.mpNrmMtxArr[side]!=&J3DMtxBuffer::sNoUseNrmMtxPtr){for(u32 view=0;view<views;++view)if(buffer.mpNrmMtxArr[side][view])JKRHeap::free(buffer.mpNrmMtxArr[side][view],heap);delete[] buffer.mpNrmMtxArr[side];}
        }
        delete[] buffer.mpAnmMtx;delete[] buffer.mpScaleFlagArr;delete[] buffer.mpWeightEvlpMtx;delete[] buffer.mpEvlpScaleFlagArr;buffer.initialize();
    }
    ~NativeMatrixBufferResource(){release();}
    NativeMatrixBufferResource(const NativeMatrixBufferResource&)=delete;
    NativeMatrixBufferResource& operator=(const NativeMatrixBufferResource&)=delete;
};

