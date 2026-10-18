#include <Game/LiveActor/LiveActorFlag.hpp>
#include <Game/Animation/XanimeCore.hpp>
#include <Game/Util/LiveActorUtil.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>



int main(){
LiveActorFlag flags;assert(flags.mIsDead&&flags.mIsNoBind&&!flags.mIsStoppedAnim&&!flags.mIsNoCalcAnim&&!flags.mIsNoCalcView&&!flags.mIsHiddenModel&&!flags.mIsCalcGravity&&!flags.mIsClipped&&!flags.mIsInvalidClipping);
TVec3f vector(3,4,5),up(0,1,0),projected;projected.killElement(vector,up);assert(projected.x==3&&projected.y==0&&projected.z==5);vector.killElement(vector,up);assert(vector.x==3&&vector.y==0&&vector.z==5);
TVec3f nonunit(0,2,0);projected.killElement(TVec3f(3,4,5),nonunit);assert(projected.y==-12);
struct Q{float x,y,z,w;};TPos3f matrix;matrix.setQT(Q{0,0,0,1},TVec3f(5,-7,9));TVec3f result;matrix.mult(TVec3f(2,0,0),result);assert((result-TVec3f(7,-7,9)).length()<.0001f);
const float h=std::sqrt(.5f);matrix.setQT(Q{0,0,h,h},TVec3f(5,-7,9));matrix.mult(TVec3f(2,0,0),result);assert((result-TVec3f(5,-5,9)).length()<.0001f);
matrix.setQT(Q{0,0,1,1},TVec3f(0,0,0));assert(matrix.mMtx[0][0]==-1&&matrix.mMtx[0][1]==-2&&matrix.mMtx[1][0]==2);
static_assert(sizeof(decltype(XjointTransform::_68))==sizeof(std::uintptr_t));XjointTransform joint;assert(joint._68==0);
TPos3f affine,inverse,product;affine.set(2,1,0,7,0,3,1,-5,1,0,-4,9);assert(inverse.invert(affine));
product.concat(affine,inverse);for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(std::fabs(product(r,c)-(r==c?1.0f:0.0f))<.00001f);
TVec3f point(3,-2,6),mapped,restored;affine.mult(point,mapped);inverse.mult(mapped,restored);assert((restored-point).length()<.00001f);
TPos3f aliased=affine;assert(aliased.invert(aliased));for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(aliased(r,c)==inverse(r,c));
TPos3f singular;singular.identity();singular.mMtx[2][2]=0;TPos3f unchanged=inverse;assert(!inverse.invert(singular));for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(inverse(r,c)==unchanged(r,c));
const float cutoff=32*std::numeric_limits<float>::epsilon();singular.mMtx[2][2]=cutoff;assert(!inverse.invert(singular));singular.mMtx[2][2]=-cutoff;assert(!inverse.invert(singular));singular.mMtx[2][2]=2*cutoff;assert(inverse.invert(singular));
std::puts("PASS original actor flags, projection/quaternion/affine inverse transforms and native joint pointer initialization");}

