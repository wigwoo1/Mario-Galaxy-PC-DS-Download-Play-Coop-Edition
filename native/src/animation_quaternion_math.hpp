#pragma once
#include <revolution/types.h>
#include <cmath>
struct NativeAnimationQuaternion {float x=0,y=0,z=0,w=1;};
inline float nativeJmaSin(s16 angle){return float(std::sin((u16(angle)>>2)*(6.28318530717958647692/16384)));}
inline float nativeJmaCos(s16 angle){return float(std::cos((u16(angle)>>2)*(6.28318530717958647692/16384)));}
// Original JMAEulerToQuat formula; native trig preserves 14-bit lookup quantization.
inline NativeAnimationQuaternion nativeAnimationQuaternion(s16 x,s16 y,s16 z){
 float cosX=nativeJmaCos(x/2),cosY=nativeJmaCos(y/2),cosZ=nativeJmaCos(z/2),sinX=nativeJmaSin(x/2),sinY=nativeJmaSin(y/2),sinZ=nativeJmaSin(z/2);float cyz=cosY*cosZ,syz=sinY*sinZ;NativeAnimationQuaternion q;q.w=cosX*cyz+sinX*syz;q.x=sinX*cyz-cosX*syz;q.y=cosZ*(cosX*sinY)+sinZ*(sinX*cosY);q.z=sinZ*(cosX*cosY)-cosZ*(sinX*sinY);return q;
}
// Original JMAQuatLerp branches with scalar dot product replacing PPC paired-single assembly.
inline NativeAnimationQuaternion lerpAnimationQuaternion(const NativeAnimationQuaternion& p,const NativeAnimationQuaternion& q,float t){
 assetCheck(std::isfinite(t)&&t>=0&&t<=1,"Invalid quaternion blend weight");float dp=p.x*q.x+p.y*q.y+p.z*q.z+p.w*q.w;assetCheck(std::isfinite(dp),"Invalid quaternion blend input");NativeAnimationQuaternion out;if(dp<0){out.x=-t*(p.x+q.x)+p.x;out.y=-t*(p.y+q.y)+p.y;out.z=-t*(p.z+q.z)+p.z;out.w=-t*(p.w+q.w)+p.w;}else{out.x=-t*(p.x-q.x)+p.x;out.y=-t*(p.y-q.y)+p.y;out.z=-t*(p.z-q.z)+p.z;out.w=-t*(p.w-q.w)+p.w;}return out;
}
inline std::array<float,12> animationQuaternionMatrix(const NativeAnimationQuaternion& q){float n=q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w;assetCheck(std::isfinite(n)&&n>1e-12,"Invalid quaternion rotation norm");float s=2/n,x=q.x,y=q.y,z=q.z,w=q.w;return {1-s*(y*y+z*z),s*(x*y-z*w),s*(x*z+y*w),0,s*(x*y+z*w),1-s*(x*x+z*z),s*(y*z-x*w),0,s*(x*z-y*w),s*(y*z+x*w),1-s*(x*x+y*y),0};}


