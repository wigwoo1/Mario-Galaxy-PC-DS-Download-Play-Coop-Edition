#pragma once
#include "native_model_resource.hpp"
#include "native_model_env_init.hpp"
#include <JSystem/JKernel/JKRArchive.hpp>
// Original model-data interface with retained native resources behind its virtual lifetime.
class NativeOwnedModelData final : public J3DModelData {
public:
    NativeModelResource resources;
    void downFracVtx(){
        auto& vertex=getVertexData();GXVtxAttrFmtList* position=nullptr;
        for(auto& format:resources.vertices->formats)if(format.attr==GX_VA_POS){position=&format;break;}
        assetCheck(position!=nullptr,"Missing position format for fraction reduction");if(!position->frac)return;
        assetCheck(position->type==GX_S16 && position->cnt==GX_POS_XYZ,"Fraction reduction requires S16 XYZ positions");
        auto* bytes=static_cast<u8*>(vertex.getVtxPosArray());auto& storage=resources.vertices->storage;size_t count=size_t(getVtxNum())*3;
        assetCheck(bytes!=nullptr,"Missing position array for fraction reduction");size_t offset=size_t(bytes-storage.data());extent(offset,count*2,storage.size());
        for(size_t i=0;i<count;++i){u16 bits=(u16(bytes[i*2])<<8)|bytes[i*2+1];int value=bits>=32768?int(bits)-65536:int(bits);int reduced=value>=0?value/2:-((-value+1)/2);u16 encoded=u16(reduced);bytes[i*2]=u8(encoded>>8);bytes[i*2+1]=u8(encoded);}
        --position->frac;vertex.setVtxPosFrac(position->frac);resources.vertices->data.setVtxPosFrac(position->frac);
        Native::gpuResources().refresh(bytes,count*2);resources.shapes->prepareVertexCommands(vertex);
    }
    NativeOwnedModelData(const std::vector<uint8_t>& source,JKRHeap& heap,u32 loaderFlags=0):resources(source,loaderFlags){
        resources.bind(*this);resources.registerGpu();
        struct Guard{void* position=j3dSys.getVtxPos();void* normal=j3dSys.getVtxNrm();GXColor* color=static_cast<GXColor*>(j3dSys.getVtxCol());
            ~Guard(){j3dSys.setVtxPos(position);j3dSys.setVtxNrm(normal);j3dSys.setVtxCol(color);}} guard;
        resources.vertices->bindCurrentArrays();resources.shapes->prepareVertexCommands(getVertexData());
        NativeModelInit::initEnvelopeAndEnvMapOrProjMapModelData(this,resources,heap);
        Mtx identity;PSMTXIdentity(identity);for(auto* material:resources.materialPointers)material->calc(identity);resources.prepareMaterialLists(heap);
    }
};
inline std::unique_ptr<J3DModelData> createNativeModelData(const void* input,size_t available,JKRHeap& heap,u32 loaderFlags=0){
    assetCheck(input && available>=32,"Missing or truncated model resource");auto* bytes=static_cast<const uint8_t*>(input);NativeModelBlocks checked(bytes,available);
    struct HeapScope{JKRHeap* previous;explicit HeapScope(JKRHeap& target):previous(target.becomeCurrentHeap()){}~HeapScope(){if(previous)previous->becomeCurrentHeap();else JKRHeap::sCurrentHeap=nullptr;}} heapScope(heap);
    std::vector<uint8_t> source(bytes,bytes+checked.fileSize);return std::make_unique<NativeOwnedModelData>(source,heap,loaderFlags);
}

inline std::unique_ptr<J3DModelData> loadNativeArchiveModel(const JKRArchive& archive,const void* resource,JKRHeap& heap,u32 flags=0){
    if(!resource)return nullptr;s32 available=archive.getResSize(resource);assetCheck(available>=32,"Model resource not owned by archive or truncated");
    return createNativeModelData(resource,size_t(available),heap,flags);
}



