#pragma once
#include "rig.hpp"
inline ModelRig blendDiagnosticPose(const ModelRig& from,const ModelRig& to,float weight){
    assetCheck(from.joints.size()==to.joints.size()&&std::isfinite(weight)&&weight>=0&&weight<=1,"Invalid diagnostic pose blend");auto result=to;
    for(size_t j=0;j<result.joints.size();++j)for(size_t k=0;k<3;++k){auto& a=from.joints[j];auto& b=to.joints[j];auto& out=result.joints[j];out.scale[k]=a.scale[k]+(b.scale[k]-a.scale[k])*weight;out.translation[k]=a.translation[k]+(b.translation[k]-a.translation[k])*weight;
        int delta=int(b.rotation[k])-int(a.rotation[k]);if(delta>32767)delta-=65536;if(delta<-32768)delta+=65536;int value=int(std::lround(int(a.rotation[k])+delta*weight));if(value>32767)value-=65536;if(value<-32768)value+=65536;out.rotation[k]=int16_t(value);}
    return result;
}
inline void poseBlendTests(){
    ModelRig a,b;a.joints.resize(1);b.joints.resize(1);a.joints[0].scale={1,1,1};b.joints[0].scale={3,3,3};a.joints[0].translation={0,0,0};b.joints[0].translation={10,20,30};a.joints[0].rotation={32760,0,0};b.joints[0].rotation={-32760,100,0};auto halfway=blendDiagnosticPose(a,b,.5f);
    assetCheck(halfway.joints[0].scale[0]==2&&halfway.joints[0].translation[1]==10&&halfway.joints[0].rotation[0]==-32768&&halfway.joints[0].rotation[1]==50,"Diagnostic pose interpolation/angle wrap failed");
    assetCheck(blendDiagnosticPose(a,b,0).joints[0].rotation==a.joints[0].rotation&&blendDiagnosticPose(a,b,1).joints[0].rotation==b.joints[0].rotation,"Diagnostic pose blend endpoints failed");bool rejected=false;try{blendDiagnosticPose(a,b,2);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Invalid pose blend weight accepted");
}
