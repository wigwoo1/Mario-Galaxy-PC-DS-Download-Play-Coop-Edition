#pragma once
#include "native_matrix_buffer_resource.hpp"
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
// Packet owner borrows retained model data, display lists and matrix buffer.
struct NativeModelPackets {
    J3DModel model;
    NativeModelPackets(J3DModelData& data,NativeMatrixBufferResource& matrices,u32 flags=0){
        model.mModelData=&data;model.mMtxBuffer=&matrices.buffer;model.mFlags=flags;
        try{assetCheck(model.createShapePacket(&data)==0,"Original shape packet creation failed");assetCheck(model.createMatPacket(&data,flags)==0,"Original material packet creation failed");
            model.mVertexBuffer.setVertexData(&data.getVertexData());model.prepareShapePackets();}
        catch(...){release();throw;}
    }
    void release() noexcept {delete[] model.mShapePacket;delete[] model.mMatPacket;model.mShapePacket=nullptr;model.mMatPacket=nullptr;model.mMtxBuffer=nullptr;}
    ~NativeModelPackets(){release();}
    NativeModelPackets(const NativeModelPackets&)=delete;
    NativeModelPackets& operator=(const NativeModelPackets&)=delete;
};
