#pragma once
#include "graphics.hpp"
#include <Native/GpuResources.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>
#include <JSystem/J3DGraphBase/J3DVertex.hpp>
// Retain GX array bytes in disk byte order; only host metadata is converted.
struct NativeVertexResource {
    std::vector<u8> storage;
    std::vector<GXVtxAttrFmtList> formats;
    J3DVertexData data;
    bool registered=false;
    void bindCurrentArrays(){j3dSys.setVtxPos(data.mVtxPosArray);j3dSys.setVtxNrm(data.mVtxNrmArray);j3dSys.setVtxCol(data.mVtxColorArray[0]);}
    void registerGpu(){if(!registered){Native::gpuResources().add(storage.data(),storage.size());registered=true;}}
    ~NativeVertexResource(){if(registered)Native::gpuResources().remove(storage.data());}
    explicit NativeVertexResource(BinaryView block){
        assetCheck(block.size()>=64 && block.tag(0)=="VTX1" && block.u32(4)==block.size(),"Invalid VTX1 block");
        storage.resize(block.size());for(size_t i=0;i<storage.size();++i)storage[i]=block.u8(i);
        size_t start=block.u32(8);assetCheck(start>=64 && start<block.size(),"Invalid vertex format offset");bool ended=false;
        for(size_t i=0;i<32;++i){size_t p=start+i*16;extent(p,16,block.size());u32 attr=block.u32(p);GXVtxAttrFmtList entry{};entry.attr=GXAttr(attr);
            if(attr==255){formats.push_back(entry);ended=true;break;}
            assetCheck(attr>=9 && attr<=20,"Invalid vertex format attribute");entry.cnt=GXCompCnt(block.u32(p+4));entry.type=GXCompType(block.u32(p+8));entry.frac=block.u8(p+12);formats.push_back(entry);
        }
        assetCheck(ended,"Unterminated vertex formats");data.mVtxAttrFmtList=formats.data();
        auto array=[&](unsigned field)->void*{size_t offset=block.u32(field);if(!offset)return nullptr;assetCheck(offset>=64 && offset<block.size(),"Invalid vertex array offset");return storage.data()+offset;};
        data.mVtxPosArray=array(12);data.mVtxNrmArray=array(16);data.mVtxNBTArray=array(20);
        for(unsigned i=0;i<2;++i)data.mVtxColorArray[i]=static_cast<GXColor*>(array(24+i*4));
        for(unsigned i=0;i<8;++i)data.mVtxTexCoordArray[i]=array(32+i*4);
        for(auto format:formats){if(format.attr==GX_VA_POS){data.setVtxPosType(format.type);data.setVtxPosFrac(format.frac);}if(format.attr==GX_VA_NRM){data.setVtxNrmType(format.type);data.setVtxNrmFrac(format.frac);}}
        // Match original loader allocation capacities, including its trailing element.
        auto capacity=[&](unsigned field,std::initializer_list<unsigned> ends,size_t stride)->u32{
            size_t begin=block.u32(field);if(!begin)return 0;size_t end=block.size();
            for(unsigned candidate:ends)if(block.u32(candidate)){end=block.u32(candidate);break;}
            assetCheck(end>=begin && end<=block.size(),"Invalid vertex array ordering");return u32((end-begin)/stride+1);
        };
        data.mNrmNum=capacity(16,{20,24,32},data.getVtxNrmType()==GX_F32?12:6);
        data.mColNum=capacity(24,{28,32},4);data.mTexCoordNum=capacity(32,{},8);
    }
    NativeVertexResource(const NativeVertexResource&)=delete;
    NativeVertexResource& operator=(const NativeVertexResource&)=delete;
};



