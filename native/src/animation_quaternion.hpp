#pragma once
#include "animation_quaternion_math.hpp"
inline void animationQuaternionTests(){
 for(s16 x:std::array<s16,4>{0,8192,16384,-16384})for(s16 y:std::array<s16,4>{0,8192,16384,-16384})for(s16 z:std::array<s16,4>{0,8192,16384,-16384}){JointData joint;joint.rotation={x,y,z};auto reference=jointRotationTranslation(joint),actual=animationQuaternionMatrix(nativeAnimationQuaternion(x,y,z));for(size_t k=0;k<12;++k)assetCheck(std::abs(actual[k]-reference[k])<1e-5,"Quaternion/Euler rotation mismatch");}
 NativeAnimationQuaternion identity,opposite{0,0,0,-1};auto midpoint=lerpAnimationQuaternion(identity,opposite,.5f);assetCheck(midpoint.w==1,"Antipodal quaternion blend collapsed");auto turn=nativeAnimationQuaternion(0,0,16384);auto halfway=animationQuaternionMatrix(lerpAnimationQuaternion(identity,turn,.5f));auto p=transformPoint(halfway,{1,0,0});assetCheck(std::abs(p[0]-.70710678f)<1e-5&&std::abs(p[1]-.70710678f)<1e-5,"Quaternion half-turn interpolation failed");bool rejected=false;try{animationQuaternionMatrix({0,0,0,0});}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Zero quaternion norm accepted");rejected=false;try{lerpAnimationQuaternion(identity,turn,std::numeric_limits<float>::quiet_NaN());}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Invalid quaternion weight accepted");
}

