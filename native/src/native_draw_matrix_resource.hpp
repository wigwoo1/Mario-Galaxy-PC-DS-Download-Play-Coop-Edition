#pragma once
#include "graphics.hpp"
#include <JSystem/J3DGraphBase/J3DVertex.hpp>
struct NativeDrawMatrixResource {
    std::vector<u8> flags;
    std::vector<u16> indices;
    J3DDrawMtxData data;
    NativeDrawMatrixResource(BinaryView block,size_t joints,size_t envelopes){
        assetCheck(block.size()>=20 && block.tag(0)=="DRW1" && block.u32(4)==block.size(),"Invalid DRW1 block");
        size_t count=block.u16(8),flagOffset=block.u32(12),indexOffset=block.u32(16);assetCheck(flagOffset>=20 && indexOffset>=20,"Invalid draw matrix offsets");extent(flagOffset,count,block.size());extent(indexOffset,count*2,block.size());
        assetCheck(count>=envelopes,"DRW1 count smaller than envelope count");count-=envelopes;
        flags.resize(count);indices.resize(count);size_t rigid=0;bool weighted=false;
        for(size_t i=0;i<count;++i){flags[i]=block.u8(flagOffset+i);indices[i]=block.u16(indexOffset+i*2);assetCheck(flags[i]<=1 && indices[i]<(flags[i]?envelopes:joints),"Invalid draw matrix reference");
            if(flags[i]){weighted=true;assetCheck(indices[i]==i-rigid,"Unsupported envelope draw ordering");}else{assetCheck(!weighted,"Unsupported mixed draw ordering");++rigid;}}
        data.mEntryNum=u16(count);data.mDrawFullWgtMtxNum=u16(rigid);data.mDrawMtxFlag=flags.data();data.mDrawMtxIndex=indices.data();
    }
    NativeDrawMatrixResource(const NativeDrawMatrixResource&)=delete;
    NativeDrawMatrixResource& operator=(const NativeDrawMatrixResource&)=delete;
};


