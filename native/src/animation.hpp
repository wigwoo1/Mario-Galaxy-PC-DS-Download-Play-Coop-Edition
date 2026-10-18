#pragma once
#include "skinning.hpp"
struct AnimationKey {float time,value,in,out;};
struct AnimationTrack {
    std::vector<AnimationKey> keys;
    float sample(float frame,float fallback) const {
        assetCheck(std::isfinite(frame),"Invalid animation frame");
        if(keys.empty())return fallback;
        if(keys.size()==1||frame<keys.front().time)return keys.front().value;
        if(frame>=keys.back().time)return keys.back().value;
        size_t i=1;while(frame>=keys[i].time)++i;
        auto a=keys[i-1],b=keys[i];float dt=b.time-a.time,t=(frame-a.time)/dt,t2=t*t,t3=t2*t;
        float value=(2*t3-3*t2+1)*a.value+(t3-2*t2+t)*dt*a.out+(-2*t3+3*t2)*b.value+(t3-t2)*dt*b.in;
        assetCheck(std::isfinite(value),"Non-finite animation result");return value;
    }
};
struct JointAnimation {
    uint8_t loop,rotationShift;float duration;
    std::vector<std::array<AnimationTrack,9>> joints;
    ModelRig pose(ModelRig rig,float frame) const {
        assetCheck(joints.size()==rig.joints.size(),"Animation/model joint count mismatch");
        for(auto& joint:rig.joints)joint.useQuaternion=false;
        for(size_t j=0;j<joints.size();++j)for(size_t axis=0;axis<3;++axis){
            rig.joints[j].scale[axis]=joints[j][axis*3].sample(frame,1);
            float angle=joints[j][axis*3+1].sample(frame,0);
            // Truncate before scaling, matching J3D; unsigned wrapping avoids negative shifts.
            assetCheck(std::abs(angle)<2147483647.0f,"Rotation outside integer range");
            uint32_t bits=uint32_t(int32_t(angle))*(1u<<rotationShift);
            rig.joints[j].rotation[axis]=int16_t(uint16_t(bits));
            rig.joints[j].translation[axis]=joints[j][axis*3+2].sample(frame,0);
        }
        return rig;
    }
};
inline JointAnimation decodeBck(BinaryView view){
    assetCheck(view.tag(0)=="J3D1"&&view.tag(4)=="bck1","Unsupported BCK header");
    assetCheck(view.u32(8)==view.size()&&view.u32(12)==1,"Invalid BCK container");
    auto block=view.sub(32,view.u32(36));assetCheck(block.tag(0)=="ANK1","Missing ANK1");
    JointAnimation animation{block.u8(8),block.u8(9),float(block.u16(10)),{}};
    assetCheck(animation.loop<=4&&animation.rotationShift<=15&&animation.duration<32768,"Invalid animation metadata");
    size_t count=block.u16(12),table=block.u32(20);extent(table,count*54,block.size());animation.joints.resize(count);
    for(size_t j=0;j<count;++j)for(size_t channel=0;channel<9;++channel){
        size_t kind=channel%3,entry=table+j*54+channel*6,n=block.u16(entry),offset=block.u16(entry+2),type=block.u16(entry+4);
        assetCheck(type<=1,"Invalid animation tangent type");size_t stride=type?4:3,values=n>1?n*stride:n;
        size_t available=block.u16(14+kind*2),base=block.u32(24+kind*4),width=kind==1?2:4;
        extent(base,available*width,block.size());extent(offset,values,available);
        auto read=[&](size_t index){return kind==1?float(int16_t(block.u16(base+index*width))):block.f32(base+index*width);};
        auto& keys=animation.joints[j][channel].keys;
        for(size_t k=0;k<n;++k){size_t p=offset+k*stride;AnimationKey key=n==1?AnimationKey{0,read(offset),0,0}:AnimationKey{read(p),read(p+1),read(p+2),read(p+(type?3:2))};
            assetCheck(k==0||key.time>=keys.back().time,"Animation times decrease");keys.push_back(key);}
    }
    return animation;
}
inline void animationTests(){
    AnimationTrack linear{{{0,0,1,1},{10,10,1,1}}};
    assetCheck(std::abs(linear.sample(3,0)-3)<0.0001&&linear.sample(-1,0)==0&&linear.sample(11,0)==10,"Hermite linear/clamp test");
    AnimationTrack curved{{{0,0,0,2},{1,1,-2,0}}};assetCheck(std::abs(curved.sample(.5f,0)-1)<.0001,"Separate tangent test");
    AnimationTrack discontinuous{{{0,0,0,0},{5,1,0,0},{5,2,0,0},{6,3,0,0}}};
    assetCheck(discontinuous.sample(5,0)==2&&std::isfinite(discontinuous.sample(5.5f,0)),"Duplicate-time transition test");
    AnimationTrack initialJump{{{0,1,0,0},{0,2,0,0},{1,3,0,0}}};
    assetCheck(initialJump.sample(0,0)==2,"Duplicate initial timestamp test");
    bool rejected=false;try{std::vector<uint8_t> truncated(8);decodeBck(BinaryView(truncated));}catch(const std::exception&){rejected=true;}
    assetCheck(rejected,"Truncated animation accepted");
    JointAnimation animation{2,2,10,{}};animation.joints.resize(1);animation.joints[0][1].keys={{0,-8192,0,0}};
    ModelRig rig;rig.joints.resize(1);auto pose=animation.pose(rig,0);
    assetCheck(pose.joints[0].rotation[0]==-32768&&pose.joints[0].scale[0]==1&&pose.joints[0].translation[0]==0,"Rotation/default channel test");
}

