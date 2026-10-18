#include <Game/Util/MathUtil.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 // Preserve the original facing test, including accepted negative ratios.
 TVec3f hit(7,8,9);
 assert(MR::checkHitSemilinePlane(&hit,TVec3f(1,2,5),TVec3f(0,0,-2),TVec3f(0,0,1),TVec3f(0,0,3)));
 assert(hit.x==1 && hit.y==2 && hit.z==1);
 hit.set(7,8,9);
 assert(!MR::checkHitSemilinePlane(&hit,TVec3f(1,2,5),TVec3f(0,0,2),TVec3f(0,0,1),TVec3f(0,0,3)));
 assert(hit.x==7 && hit.y==8 && hit.z==9);
 assert(MR::checkHitSemilinePlane(&hit,TVec3f(1,2,5),TVec3f(0,0,-2),TVec3f(0,0,9),TVec3f(0,0,1)));
 assert(hit.x==1 && hit.y==2 && hit.z==9);
 assert(MR::checkHitSemilinePlane(&hit,TVec3f(1,2,5),TVec3f(0,0,-2),TVec3f(0,0,5),TVec3f(0,0,1)));
 assert(hit.x==1 && hit.y==2 && hit.z==5);
 // Original parallel inputs return true with nonfinite output; callers must avoid them.
 assert(MR::checkHitSemilinePlane(&hit,TVec3f(0,0,0),TVec3f(1,0,0),TVec3f(0,0,1),TVec3f(0,0,1)));
 assert(!std::isfinite(hit.x) && !std::isfinite(hit.z));
 puts("PASS original ray-plane facing/scaled normals/rear plane/on-plane and audited parallel behavior");
 TVec2f pointer(3,-4);
 assert(!MR::normalizeOrZero(&pointer));
 assert(std::abs(pointer.x-0.6f)<0.0001f && std::abs(pointer.y+0.8f)<0.0001f);
 pointer.set(0.001f,-0.001f); assert(MR::normalizeOrZero(&pointer));
 assert(pointer.x==0 && pointer.y==0);
 pointer.set(0.00101f,0.0f); assert(!MR::normalizeOrZero(&pointer));
 assert(std::abs(pointer.x-1)<0.0001f && pointer.y==0);
 pointer.set(0,0); assert(MR::normalizeOrZero(&pointer));
 puts("PASS original pointer normalization signed vector/zero/inclusive threshold/outside threshold");
 float magnitude=-1; TVec2f direction(9,9),source(-3,4);
 MR::separateScalarAndDirection(&magnitude,&direction,source);
 assert(magnitude==5 && std::abs(direction.x+0.6f)<0.0001f && std::abs(direction.y-0.8f)<0.0001f);
 assert(source.x==-3 && source.y==4);
 MR::separateScalarAndDirection(&magnitude,&source,source);
 assert(magnitude==5 && std::abs(source.x+0.6f)<0.0001f && std::abs(source.y-0.8f)<0.0001f);
 source.set(0.001f,-0.001f);MR::separateScalarAndDirection(&magnitude,&direction,source);
 assert(magnitude>0 && std::abs(magnitude-std::sqrt(0.000002f))<0.0000001f);
 assert(direction.x==0 && direction.y==0);
 source.set(0.0f,0.0f);MR::separateScalarAndDirection(&magnitude,&direction,source);
 assert(magnitude==0 && direction.x==0 && direction.y==0);
 source.set(-0.00101f,0.0f);MR::separateScalarAndDirection(&magnitude,&direction,source);
 assert(std::abs(magnitude-0.00101f)<0.0000001f && std::abs(direction.x+1)<0.0001f);
 puts("PASS original pointer magnitude/direction signed vector/source retention/alias/near-zero magnitude");
 assert(MR::normalizeAbs(5,10,20)==0);
 assert(MR::normalizeAbs(10,10,20)==0);
 assert(MR::normalizeAbs(15,10,20)==0.5f);
 assert(MR::normalizeAbs(-15,10,20)==-0.5f);
 assert(MR::normalizeAbs(20,10,20)==1 && MR::normalizeAbs(-30,10,20)==-1);
 assert(MR::normalizeAbs(4,5,5)==0 && MR::normalizeAbs(5,5,5)==1);
 assert(MR::normalizeAbs(-5,5,5)==-1);
 puts("PASS original signed angle range mapping: dead zone/midpoint/saturation/zero-width range");
 TVec3f a(-2,0,0),b(2,0,0),d;
 assert(MR::checkHitSegmentSphere(TVec3f(0,0,0),a,b,1,nullptr));
 assert(MR::checkHitSegmentSphere(TVec3f(0,1,0),a,b,1,&d));
 assert(std::abs(d.y-1)<0.0001f && std::abs(d.x)<0.0001f);
 assert(!MR::checkHitSegmentSphere(TVec3f(0,1.001f,0),a,b,1,nullptr));
 assert(MR::checkHitSegmentSphere(TVec3f(-2.5f,0,0),a,b,1,&d));
 assert(std::abs(d.x+1)<0.0001f);
 assert(!MR::checkHitSegmentSphere(TVec3f(-3,0,0),a,b,1,nullptr));
 assert(MR::checkHitSegmentSphere(TVec3f(2.5f,0,0),a,b,1,&d));
 assert(std::abs(d.x-1)<0.0001f);
 assert(!MR::checkHitSegmentSphere(TVec3f(3,0,0),a,b,1,nullptr));
 d.set(7,8,9);assert(!MR::checkHitSegmentSphere(TVec3f(0,2,0),a,b,1,&d));
 assert(d.x==7 && d.y==8 && d.z==9);
 assert(std::abs(MR::convergeRadian(1,2,0.25f)-1.25f)<0.001f);
 assert(std::abs(MR::convergeRadian(2,1,0.25f)-1.75f)<0.001f);
 assert(std::abs(MR::convergeRadian(1,1.1f,0.25f)-1.1f)<0.001f);
 assert(std::abs(MR::mod(13,5)-3)<0.001f);assert(std::abs(MR::mod(-13,5)+3)<0.001f);assert(std::abs(MR::blendAngle(0,1,0.5f)-0.5f)<0.001f);assert(std::abs(MR::blendAngle(6.083185307f,0.2f,0.5f)-MR::pi2())<0.001f);
 puts("PASS original camera segment-sphere interiors/tangents/endpoint rules/directions/miss output and angle convergence");
}







