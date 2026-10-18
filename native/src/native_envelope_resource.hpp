#pragma once
#include "graphics.hpp"
#include <JSystem/J3DGraphAnimator/J3DJointTree.hpp>
#include <set>
// Native metadata and inverse bind matrices retained for original skinning code.
struct NativeEnvelopeResource {
    std::vector<u8> counts;
    std::vector<u16> indices;
    std::vector<float> weights;
    std::vector<std::array<float,12>> inverseBind;
    NativeEnvelopeResource(BinaryView block,size_t joints){
        assetCheck(block.size()>=28 && block.tag(0)=="EVP1" && block.u32(4)==block.size(),"Invalid EVP1 block");
        size_t count=block.u16(8),countOffset=block.u32(12),indexOffset=block.u32(16),weightOffset=block.u32(20),inverseOffset=block.u32(24);
        if(!count)return;
        assetCheck(countOffset>=28 && indexOffset>=28 && weightOffset>=28,"Invalid envelope offsets");extent(countOffset,count,block.size());
        counts.resize(count);size_t total=0;for(size_t i=0;i<count;++i){counts[i]=block.u8(countOffset+i);assetCheck(counts[i]>0,"Empty skin envelope");total+=counts[i];}
        extent(indexOffset,total*2,block.size());extent(weightOffset,total*4,block.size());indices.resize(total);weights.resize(total);
        std::set<u16> used;size_t cursor=0;
        for(auto entries:counts){double sum=0;for(unsigned k=0;k<entries;++k,++cursor){indices[cursor]=block.u16(indexOffset+cursor*2);weights[cursor]=block.f32(weightOffset+cursor*4);
            assetCheck(indices[cursor]<joints && std::isfinite(weights[cursor]) && weights[cursor]>=0 && weights[cursor]<=1,"Invalid skin weight");sum+=weights[cursor];used.insert(indices[cursor]);}
            assetCheck(std::fabs(sum-1)<0.001,"Skin weights do not sum to one");}
        assetCheck(inverseOffset>=28,"Missing inverse bind matrices");inverseBind.resize(size_t(*used.rbegin())+1);extent(inverseOffset,inverseBind.size()*48,block.size());
        for(u16 joint:used)for(unsigned k=0;k<12;++k){float value=block.f32(inverseOffset+size_t(joint)*48+k*4);assetCheck(std::isfinite(value),"Invalid inverse bind matrix");inverseBind[joint][k]=value;}
    }
    void bind(J3DJointTree& tree){tree.mWEvlpMtxNum=u16(counts.size());tree.mWEvlpMixMtxNum=counts.data();tree.mWEvlpMixMtxIndex=indices.data();tree.mWEvlpMixWeight=weights.data();tree.mInvJointMtx=reinterpret_cast<Mtx*>(inverseBind.data());}
    NativeEnvelopeResource(const NativeEnvelopeResource&)=delete;
    NativeEnvelopeResource& operator=(const NativeEnvelopeResource&)=delete;
};
