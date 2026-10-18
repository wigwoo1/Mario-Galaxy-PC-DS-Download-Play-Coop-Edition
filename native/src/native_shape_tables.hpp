#pragma once
#include "graphics.hpp"
#include <JSystem/J3DGraphLoader/J3DShapeFactory.hpp>
// Native SHP1 initialization records; additional geometry tables follow separately.
struct NativeShapeTables {
    std::vector<J3DShapeInitData> records;
    std::vector<u16> indices,matrixIndices;
    std::vector<J3DShapeMtxInitData> matrixRecords;
    std::vector<J3DShapeDrawInitData> drawRecords;
    std::vector<u8> displayLists;
    std::vector<GXVtxDescList> descriptors;
    explicit NativeShapeTables(BinaryView block){
        assetCheck(block.size()>=44 && block.tag(0)=="SHP1" && block.u32(4)==block.size(),"Invalid SHP1 block");
        size_t count=block.u16(8),init=block.u32(12),remap=block.u32(16);
        assetCheck(init>=44 && remap>=44,"Invalid shape table offsets");extent(init,count*40,block.size());extent(remap,count*2,block.size());
        records.resize(count);indices.resize(count);
        for(size_t i=0;i<count;++i){indices[i]=block.u16(remap+i*2);assetCheck(indices[i]<count,"Invalid shape remap");size_t p=init+i*40;auto& record=records[i];
            record.mShapeMtxType=block.u8(p);assetCheck(record.mShapeMtxType<=3,"Invalid shape matrix type");record.mMtxGroupNum=block.u16(p+2);record.mVtxDescListIndex=block.u16(p+4);
            record.mMtxInitDataIndex=block.u16(p+6);record.mDrawInitDataIndex=block.u16(p+8);record.mRadius=block.f32(p+12);
            for(unsigned k=0;k<3;++k){(&record.mMin.x)[k]=block.f32(p+16+k*4);(&record.mMax.x)[k]=block.f32(p+28+k*4);}
        }
    }
    void decodeGeometryTables(BinaryView block){
        size_t matrices=block.u32(28),lists=block.u32(32),matrixInit=block.u32(36),drawInit=block.u32(40);
        for(size_t offset:{matrices,lists,matrixInit,drawInit})assetCheck(offset>=44 && offset<=block.size(),"Invalid shape geometry offset");
        static_assert(sizeof(GXVtxDescList)==8);size_t descOffset=block.u32(24);assetCheck(descOffset>=44 && descOffset<=block.size(),"Invalid descriptor offset");
        size_t descCount=0;
        for(auto& record:records){assetCheck(record.mVtxDescListIndex%8==0,"Unaligned shape descriptor");size_t first=record.mVtxDescListIndex/8;bool ended=false;
            for(size_t j=0;j<32;++j){size_t p=descOffset+(first+j)*8;extent(p,8,block.size());u32 attr=block.u32(p),kind=block.u32(p+4);
                assetCheck(attr==255 || (attr<=20 && kind<=3),"Invalid shape descriptor");descCount=std::max(descCount,first+j+1);if(attr==255){ended=true;break;}}
            assetCheck(ended,"Unterminated shape descriptors");}
        descriptors.resize(descCount);for(size_t i=0;i<descCount;++i){descriptors[i].attr=GXAttr(block.u32(descOffset+i*8));descriptors[i].type=GXAttrType(block.u32(descOffset+i*8+4));}
        size_t matrixCount=0,drawCount=0;
        for(auto& record:records){matrixCount=std::max(matrixCount,size_t(record.mMtxInitDataIndex)+record.mMtxGroupNum);drawCount=std::max(drawCount,size_t(record.mDrawInitDataIndex)+record.mMtxGroupNum);}
        extent(matrixInit,matrixCount*8,block.size());extent(drawInit,drawCount*8,block.size());matrixRecords.resize(matrixCount);drawRecords.resize(drawCount);
        size_t indexCount=0;
        for(size_t i=0;i<matrixCount;++i){size_t p=matrixInit+i*8;auto& record=matrixRecords[i];record.mUseMtxIndex=block.u16(p);record.mUseMtxCount=block.u16(p+2);record.mFirstUseMtxIndex=block.u32(p+4);indexCount=std::max(indexCount,size_t(record.mFirstUseMtxIndex)+record.mUseMtxCount);}
        extent(matrices,indexCount*2,block.size());matrixIndices.resize(indexCount);for(size_t i=0;i<indexCount;++i)matrixIndices[i]=block.u16(matrices+i*2);
        size_t listEnd=block.size();for(unsigned field=12;field<=40;field+=4){size_t offset=block.u32(field);if(offset>lists)listEnd=std::min(listEnd,offset);}
        displayLists.resize(listEnd-lists);for(size_t i=0;i<displayLists.size();++i)displayLists[i]=block.u8(lists+i);
        for(size_t i=0;i<drawCount;++i){size_t p=drawInit+i*8;drawRecords[i].mDisplayListSize=block.u32(p);drawRecords[i].mDisplayListIndex=block.u32(p+4);extent(drawRecords[i].mDisplayListIndex,drawRecords[i].mDisplayListSize,displayLists.size());}
    }
    void bindGeometry(J3DShapeFactory& factory){bindRecords(factory);factory.mVtxDescList=descriptors.data();factory.mMtxTable=matrixIndices.data();factory.mDisplayListData=displayLists.data();factory.mMtxInitData=matrixRecords.data();factory.mDrawInitData=drawRecords.data();}
    void bindRecords(J3DShapeFactory& factory){factory.mShapeInitData=records.data();factory.mIndexTable=indices.data();}
};


