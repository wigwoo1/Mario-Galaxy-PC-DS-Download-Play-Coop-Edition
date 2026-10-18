#pragma once
#include "native_model_resource.hpp"
#include "native_model_env_init.hpp"
#include "native_model_packets.hpp"
// Destruction order keeps every original borrowed pointer alive until its user is gone.
struct NativeModelInstance {
    NativeModelResource resources;
    J3DModelData data;
    std::unique_ptr<NativeMatrixBufferResource> matrices;
    std::unique_ptr<NativeModelPackets> packets;
    explicit NativeModelInstance(const std::vector<uint8_t>& source,JKRHeap& heap,u32 views=1,u32 loaderFlags=0):resources(source,loaderFlags){
        resources.bind(data);resources.registerGpu();
        struct ArrayGuard{void* position=j3dSys.getVtxPos();void* normal=j3dSys.getVtxNrm();GXColor* color=static_cast<GXColor*>(j3dSys.getVtxCol());
            ~ArrayGuard(){j3dSys.setVtxPos(position);j3dSys.setVtxNrm(normal);j3dSys.setVtxCol(color);}} guard;
        resources.vertices->bindCurrentArrays();resources.shapes->prepareVertexCommands(data.getVertexData());
        NativeModelInit::initEnvelopeAndEnvMapOrProjMapModelData(&data,resources,heap);
        Mtx identity;PSMTXIdentity(identity);for(auto* material:resources.materialPointers)material->calc(identity);
        resources.prepareMaterialLists(heap);matrices=std::make_unique<NativeMatrixBufferResource>(data,views);packets=std::make_unique<NativeModelPackets>(data,*matrices);
    }
    J3DModel& model(){return packets->model;}
    void calculate(J3DMtxCalc& animation,const Vec& scale,const Mtx& transform){
        data.setBasicMtxCalc(&animation);model().mBaseScale=scale;PSMTXCopy(transform,model().mBaseTransformMtx);model().calc();
    }
    void calculateView(u32 view){assetCheck(view<matrices->views,"Model view index out of range");matrices->buffer.mCurrentViewNo=view;model().viewCalc();matrices->refreshGpuMatrices();}
    NativeModelInstance(const NativeModelInstance&)=delete;
    NativeModelInstance& operator=(const NativeModelInstance&)=delete;
};



