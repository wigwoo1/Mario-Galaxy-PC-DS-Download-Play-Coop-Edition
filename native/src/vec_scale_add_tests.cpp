#include <JSystem/JGeometry/TVec.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
namespace MR { void vecScaleAdd(const TVec3f*,const TVec3f*,f32); }
int main(){
TVec3f a(1,-2,4),b(3,4,-6);MR::vecScaleAdd(&a,&b,.5f);assert(a.x==2.5f&&a.y==0&&a.z==1);assert(b.x==3&&b.y==4&&b.z==-6);
MR::vecScaleAdd(&a,&a,-1);assert(a.x==0&&a.y==0&&a.z==0);
a.set(-1,0,0);b.set(1+std::ldexp(1.0f,-23),0.0f,0.0f);MR::vecScaleAdd(&a,&b,1-std::ldexp(1.0f,-23));assert(a.x==-std::ldexp(1.0f,-46));
std::puts("PASS vector scale-add: accumulation, source preservation, aliasing and fused rounding");
}

