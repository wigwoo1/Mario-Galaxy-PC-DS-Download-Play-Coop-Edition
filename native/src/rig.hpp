#pragma once
#include "graphics.hpp"
struct JointData {
    int parent=-1;
    std::array<float,3> scale{},translation{};
    std::array<int16_t,3> rotation{};
    bool scaleCompensate=false;
    bool useQuaternion=false;std::array<float,4> quaternion{0,0,0,1};
};
struct SkinWeight { uint16_t joint;float weight; };
struct DrawMatrix { bool weighted;uint16_t index; };
struct ModelRig {
    uint16_t matrixMode=0;
    std::vector<JointData> joints;
    std::vector<std::vector<SkinWeight>> envelopes;
    std::vector<DrawMatrix> drawMatrices;
    std::vector<std::array<float,12>> inverseBind;
    std::vector<bool> inverseBindPresent;
};
inline ModelRig decodeRig(BinaryView file) {
    assetCheck(file.size()>=32&&file.tag(0)=="J3D2"&&file.u32(8)==file.size(),"Invalid rig container");
    std::map<std::string,BinaryView> sections;size_t pos=32;
    uint32_t count=file.u32(12);assetCheck(count<=64,"Invalid rig section count");
    for(uint32_t i=0;i<count;++i){size_t n=file.u32(pos+4);assetCheck(n>=8,"Invalid rig section size");assetCheck(sections.emplace(file.tag(pos),file.sub(pos,n)).second,"Duplicate rig section");pos+=n;}
    assetCheck(pos==file.size()&&sections.count("INF1")&&sections.count("JNT1"),"Incomplete rig container");
    ModelRig rig;auto joint=sections.at("JNT1");uint16_t num=joint.u16(8);size_t init=joint.u32(12),remap=joint.u32(16);
    extent(remap,size_t(num)*2,joint.size());rig.joints.resize(num);
    for(uint16_t i=0;i<num;++i){
        uint16_t index=joint.u16(remap+i*2);assetCheck(index<num,"Invalid joint remap");size_t p=init+size_t(index)*64;extent(p,64,joint.size());
        auto& j=rig.joints[i];j.scaleCompensate=joint.u8(p+2)==1;
        for(size_t k=0;k<3;++k){j.scale[k]=joint.f32(p+4+k*4);j.rotation[k]=int16_t(joint.u16(p+16+k*2));j.translation[k]=joint.f32(p+24+k*4);}
    }
    auto info=sections.at("INF1");rig.matrixMode=info.u16(8)&15;assetCheck(rig.matrixMode<=2,"Unsupported joint transform mode");size_t hierarchy=info.u32(20);std::vector<int> stack;int parent=-1,last=-1;std::set<uint16_t> seen;bool ended=false;
    for(size_t p=hierarchy;p+4<=info.size();p+=4){
        uint16_t type=info.u16(p),index=info.u16(p+2);
        if(type==0){assetCheck(stack.empty(),"Unbalanced joint hierarchy");ended=true;break;}
        if(type==1){assetCheck(last>=0&&stack.size()<128,"Invalid hierarchy open");stack.push_back(parent);parent=last;}
        else if(type==2){assetCheck(!stack.empty(),"Hierarchy stack underflow");parent=stack.back();stack.pop_back();last=parent;}
        else if(type==16){assetCheck(index<num&&seen.insert(index).second,"Invalid or duplicate hierarchy joint");rig.joints[index].parent=parent;last=index;}
        else assetCheck(type==17||type==18,"Unknown hierarchy command");
    }
    assetCheck(ended&&seen.size()==num,"Missing hierarchy joints or terminator");
    if(sections.count("EVP1")){
        auto ev=sections.at("EVP1");uint16_t n=ev.u16(8);size_t counts=ev.u32(12),indices=ev.u32(16),weights=ev.u32(20),inverse=ev.u32(24);
        extent(counts,n,ev.size());size_t total=0;for(size_t i=0;i<n;++i)total+=ev.u8(counts+i);
        extent(indices,total*2,ev.size());extent(weights,total*4,ev.size());size_t cursor=0;uint16_t highest=0;std::set<uint16_t> usedJoints;
        for(size_t i=0;i<n;++i){
            uint8_t entries=ev.u8(counts+i);assetCheck(entries>0,"Empty skin envelope");std::vector<SkinWeight> envelope;double sum=0;
            for(size_t k=0;k<entries;++k,++cursor){uint16_t j=ev.u16(indices+cursor*2);float w=ev.f32(weights+cursor*4);assetCheck(j<num&&w>=0&&w<=1,"Invalid skin weight");envelope.push_back({j,w});sum+=w;highest=std::max(highest,j);usedJoints.insert(j);}
            assetCheck(std::abs(sum-1)<0.001,"Skin weights do not sum to one");rig.envelopes.push_back(std::move(envelope));
        }
        if(total){assetCheck(inverse!=0,"Missing inverse bind matrices");extent(inverse,(size_t(highest)+1)*48,ev.size());
            rig.inverseBind.resize(size_t(highest)+1);rig.inverseBindPresent.resize(size_t(highest)+1,false);
            // Unreferenced slots may contain exporter padding, including NaNs.
            // Only referenced matrices are meaningful and must be finite.
            for(uint16_t i:usedJoints){for(size_t k=0;k<12;++k)rig.inverseBind[i][k]=ev.f32(inverse+size_t(i)*48+k*4);rig.inverseBindPresent[i]=true;}}
    }
    if(sections.count("DRW1")){
        auto dr=sections.at("DRW1");uint16_t n=dr.u16(8);size_t flags=dr.u32(12),indices=dr.u32(16);extent(flags,n,dr.size());extent(indices,size_t(n)*2,dr.size());
        for(size_t i=0;i<n;++i){uint8_t flag=dr.u8(flags+i);uint16_t index=dr.u16(indices+i*2);assetCheck(flag<=1&&index<(flag?rig.envelopes.size():rig.joints.size()),"Invalid draw matrix reference");rig.drawMatrices.push_back({flag!=0,index});}
    }
    return rig;
}

