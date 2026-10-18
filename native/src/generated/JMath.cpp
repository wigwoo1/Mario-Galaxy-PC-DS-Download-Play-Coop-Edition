#include "JSystem/JMath/JMath.hpp"
#include "JSystem/JMath/JMATrigonometric.hpp"

void JMAEulerToQuat(s16 x, s16 y, s16 z, Quaternion* quat) {
    f32 cosX = JMASCos(x / 2);
    f32 cosY = JMASCos(y / 2);
    f32 cosZ = JMASCos(z / 2);
    f32 sinX = JMASSin(x / 2);
    f32 sinY = JMASSin(y / 2);
    f32 sinZ = JMASSin(z / 2);

    f32 cyz = cosY * cosZ;
    f32 syz = sinY * sinZ;
    quat->w = cosX * (cyz) + sinX * (syz);
    quat->x = sinX * (cyz)-cosX * (syz);
    quat->y = cosZ * (cosX * sinY) + sinZ * (sinX * cosY);
    quat->z = sinZ * (cosX * cosY) - cosZ * (sinX * sinY);
}

void JMAQuatLerp(const Quaternion* p,const Quaternion* q,f32 t,Quaternion* dst){
    const Quaternion a=*p,b=*q;
    const f32 dot=a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
    const f32 sign=dot<0?-1.0f:1.0f;
    dst->x=a.x+t*(sign*b.x-a.x);dst->y=a.y+t*(sign*b.y-a.y);
    dst->z=a.z+t*(sign*b.z-a.z);dst->w=a.w+t*(sign*b.w-a.w);
}
void JMAVECScaleAdd(const Vec* a,const Vec* b,Vec* dst,f32 scale){
    const Vec result{a->x*scale+b->x,a->y*scale+b->y,a->z*scale+b->z};*dst=result;
}
void JMAVECLerp(const Vec* a,const Vec* b,Vec* dst,f32 t){
    const Vec result{a->x+(b->x-a->x)*t,a->y+(b->y-a->y)*t,a->z+(b->z-a->z)*t};*dst=result;
}
void JMAMTXApplyScale(const Mtx src,Mtx dst,f32 x,f32 y,f32 z){
    Mtx result;
    for(int r=0;r<3;++r){result[r][0]=src[r][0]*x;result[r][1]=src[r][1]*y;result[r][2]=src[r][2]*z;result[r][3]=src[r][3];}
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)dst[r][c]=result[r][c];
}