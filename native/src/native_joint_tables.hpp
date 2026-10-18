#pragma once
#include "graphics.hpp"
#include "native_name_table.hpp"
#include <JSystem/J3DGraphLoader/J3DJointFactory.hpp>
#include <JSystem/J3DGraphLoader/J3DModelLoader.hpp>
// Own native-endian JNT1 records; factory pointers remain valid while this owner lives.
struct NativeJointTables {
    std::vector<J3DJointInitData> records;
    std::vector<u16> indices;
    std::unique_ptr<NativeNameTable> names;
    explicit NativeJointTables(BinaryView block){
        assetCheck(block.size()>=24 && block.tag(0)=="JNT1" && block.u32(4)==block.size(),"Invalid JNT1 block");
        size_t count=block.u16(8),init=block.u32(12),remap=block.u32(16);
        assetCheck(init>=24 && remap>=24,"Invalid JNT1 table offset");
        extent(init,count*64,block.size());extent(remap,count*2,block.size());records.resize(count);indices.resize(count);
        size_t nameOffset=block.u32(20);if(nameOffset){assetCheck(nameOffset>=24 && nameOffset<block.size(),"Invalid joint name offset");names=std::make_unique<NativeNameTable>(block.sub(nameOffset,block.size()-nameOffset));assetCheck(names->resource()->mEntryNum==count,"Joint name count mismatch");}
        for(size_t i=0;i<count;++i){indices[i]=block.u16(remap+i*2);assetCheck(indices[i]<count,"Invalid JNT1 remap");
            size_t p=init+i*64;auto& record=records[i];record.mKind=block.u16(p);
            u8 compensate=block.u8(p+2);assetCheck(compensate==0 || compensate==1 || compensate==255,"Invalid JNT1 compensation");record.mScaleCompensate=compensate==1;
            for(unsigned k=0;k<3;++k){(&record.mTransformInfo.mScale.x)[k]=block.f32(p+4+k*4);(&record.mTransformInfo.mRotation.x)[k]=s16(block.u16(p+16+k*2));
                (&record.mTransformInfo.mTranslate.x)[k]=block.f32(p+24+k*4);(&record.mMin.x)[k]=block.f32(p+40+k*4);(&record.mMax.x)[k]=block.f32(p+52+k*4);}
            record.mRadius=block.f32(p+36);
        }
    }
    // Contiguous native block allows unchanged original offset conversion and readJoint.
    std::vector<uint64_t> loaderStorage;
    const J3DJointBlock* loaderBlock(){
        size_t recordOffset=(sizeof(J3DJointBlock)+alignof(J3DJointInitData)-1)&~(alignof(J3DJointInitData)-1);
        size_t indexOffset=recordOffset+records.size()*sizeof(J3DJointInitData);
        size_t nameOffset=indexOffset+indices.size()*sizeof(u16);
        size_t total=nameOffset+(names?names->storage.size()*sizeof(u16):0);
        loaderStorage.assign((total+7)/8,0);auto* bytes=reinterpret_cast<uint8_t*>(loaderStorage.data());
        J3DJointBlock header{};header.mBlockType=0x4a4e5431;header.mBlockSize=u32(total);header.mJointNum=u16(indices.size());
        header.mpJointInitData=reinterpret_cast<void*>(recordOffset);header.mpIndexTable=reinterpret_cast<void*>(indexOffset);
        header.mpNameTable=names?reinterpret_cast<void*>(nameOffset):nullptr;
        std::memcpy(bytes,&header,sizeof(header));std::memcpy(bytes+recordOffset,records.data(),records.size()*sizeof(J3DJointInitData));
        std::memcpy(bytes+indexOffset,indices.data(),indices.size()*sizeof(u16));
        if(names)std::memcpy(bytes+nameOffset,names->storage.data(),names->storage.size()*sizeof(u16));
        return reinterpret_cast<const J3DJointBlock*>(bytes);
    }
    void bind(J3DJointFactory& factory){factory.mJointInitData=records.data();factory.mIndexTable=indices.data();}
};



