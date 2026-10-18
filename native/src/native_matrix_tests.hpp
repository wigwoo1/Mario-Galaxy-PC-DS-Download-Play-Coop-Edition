#pragma once
#include "JSystem/JGeometry/TMatrix.hpp"
#include <Native/AngleConversion.hpp>
#include <cmath>
#include <stdexcept>
#include <iostream>
struct ATTRIBUTE_ALIGN(32) NativeSdkAlignmentCheck { u8 value; };
static_assert(alignof(NativeSdkAlignmentCheck)==32);
inline void nativeMatrixTests(){
    auto check=[](bool ok){if(!ok)throw std::runtime_error("native affine matrix regression");};
    auto same=[](const TVec3f& a,const TVec3f& b){return std::abs(a.x-b.x)<1e-5f&&std::abs(a.y-b.y)<1e-5f&&std::abs(a.z-b.z)<1e-5f;};
    const Vec sdkOriginal{2,-3,4}; const TVec3f fromSdk(sdkOriginal); check(same(fromSdk,TVec3f(2,-3,4))); check(same(fromSdk.scaleInline(-2),TVec3f(-4,6,-8))); check(same(fromSdk,TVec3f(2,-3,4)));
    check(std::abs(DEG_TO_RAD(30)-0.5235987756f)<1e-6f&&std::abs(DEG_TO_RAD(90)-1.5707963268f)<1e-6f&&DEG_TO_RAD(0)==0);int degreeInput=29;check(std::abs(DEG_TO_RAD(++degreeInput)-0.5235987756f)<1e-6f&&degreeInput==30);check(DEG_TO_RAD(-60)==-DEG_TO_RAD(60));
    TVec3f vectorA(1,2,3),vectorB(4,-5,6); check(vectorA.dot(vectorB)==12); vectorA.lerp(vectorA,vectorB,0.5f); check(same(vectorA,TVec3f(2.5f,-1.5f,4.5f))); vectorA.sub(vectorA,vectorB); check(same(vectorA,TVec3f(-1.5f,3.5f,-1.5f))); vectorA.mul(vectorA,vectorB); check(same(vectorA,TVec3f(-6,-17.5f,-9))); vectorA.negate(vectorA);check(same(vectorA,TVec3f(6,17.5f,9))); vectorA.scale(2,vectorA);check(same(vectorA,TVec3f(12,35,18))); vectorA.zero();check(same(vectorA,TVec3f(0))); u8 copySource[5]={1,2,3,4,5},copyTarget[7]={9,9,9,9,9,9,9};check(__memcpy(copyTarget+1,copySource,5)==copyTarget+1);check(copyTarget[0]==9&&copyTarget[6]==9&&copyTarget[1]==1&&copyTarget[5]==5);check(__memcpy(copyTarget,copySource,0)==copyTarget&&copyTarget[0]==9);TQuat4f zeroQuat(0.0f,0.0f);zeroQuat.normalize();check(zeroQuat.w==1&&zeroQuat.squared()==1);TQuat4f quarterZ;quarterZ.setEuler(0,0,1.57079632679f);TVec3f basis;quarterZ.getXDir(basis);check(same(basis,TVec3f(0,1,0)));TQuat4f quarterX;quarterX.setEuler(1.57079632679f,0,0);TQuat4f product;product.mult(quarterZ,quarterX);product.getYDir(basis);check(same(basis,TVec3f(0,0,1)));TQuat4f aliasQuat=quarterX;aliasQuat.mult(quarterZ);check(std::abs(aliasQuat.dot(product)-1)<1e-5f);aliasQuat.mult(aliasQuat,quarterX);TQuat4f expectedQuat;expectedQuat.mult(product,quarterX);check(std::abs(aliasQuat.dot(expectedQuat)-1)<1e-5f);product.scale(3);product.normalize(product);check(std::abs(product.squared()-1)<1e-5f);TPos3f quaternionMatrix;quaternionMatrix.identity();quaternionMatrix.setTrans(7,8,9);quarterZ.makeMtx(quaternionMatrix.toMtxPtr());quaternionMatrix.mult33(TVec3f(1,0,0),basis);check(same(basis,TVec3f(0,1,0)));quaternionMatrix.getTrans(basis);check(same(basis,TVec3f(7,8,9)));TVec3f lengthVector(3,4,0);check(lengthVector.length()==5&&lengthVector.squared()==25);check(lengthVector.distance(TVec3f(3,4,12))==12);check(lengthVector.setLength(10)==5&&same(lengthVector,TVec3f(6,8,0)));lengthVector.cross(TVec3f(1,0,0),TVec3f(0,1,0));check(same(lengthVector,TVec3f(0,0,1)));lengthVector.cross(lengthVector,TVec3f(0,1,0));check(same(lengthVector,TVec3f(-1,0,0)));TVec3f tiny(0.0001f,0,0);check(tiny.setLength(8)==0&&tiny.x==0.0001f);lengthVector.setLength(tiny,8);check(lengthVector.isZero()&&lengthVector.x==0);TVec3f divided(6,9,12);divided=divided/3;check(same(divided,TVec3f(2,3,4)));divided/=2;check(same(divided,TVec3f(1,1.5f,2)));const TVec3f sdkVector(7,8,9);const Vec* sdkPointer=sdkVector;check(sdkPointer->x==7&&sdkPointer->y==8&&sdkPointer->z==9);Vec* writablePointer=divided;check(writablePointer->x==1&&writablePointer->z==2);static_assert(sizeof(TVec3f)==sizeof(Vec)&&alignof(TVec3f)==alignof(Vec));static_assert(offsetof(TVec3f,z)==offsetof(Vec,z));Mtx sdkRotation;TVec3f sdkAxis(0,0,4);PSMTXRotAxisRad(sdkRotation,&sdkAxis,1.57079632679f);TVec3f sdkRotated(1,0,0);PSMTXMultVecSR(sdkRotation,&sdkRotated,&sdkRotated);check(same(sdkRotated,TVec3f(0,1,0)));check(sdkRotation[0][3]==0&&sdkRotation[1][3]==0&&sdkRotation[2][3]==0);sdkRotation[0][3]=100;sdkRotation[1][3]=200;sdkRotation[2][3]=300;const Vec sdkSource{1,0,0};Vec sdkResult;PSMTXMultVecSR(sdkRotation,&sdkSource,&sdkResult);check(sdkResult.y>0.999f&&std::abs(sdkResult.x)<1e-5f&&sdkResult.z==0);check(__abs(-17)==17&&__abs(0)==0&&__abs(std::numeric_limits<s32>::min())==std::numeric_limits<s32>::min());check(__fabs(-3.5)==3.5&&__fabsf(-2.5f)==2.5f&&!std::signbit(__fabsf(-0.0f)));check(__frsqrte(4)==0.5f&&std::isinf(__frsqrte(0))&&std::isnan(__frsqrte(-1)));const float rootEstimate=__frsqrte(9);const float refined=9*(0.5f*rootEstimate*(3-9*(rootEstimate*rootEstimate)));check(std::abs(refined-3)<1e-5f);TVec3f scaleAdded(1,2,3);scaleAdded.scaleAdd(2,scaleAdded);check(same(scaleAdded,TVec3f(3,6,9)));scaleAdded.scaleAdd(-2,TVec3f(0,1,0),scaleAdded);check(same(scaleAdded,TVec3f(3,4,9)));static_assert(std::is_volatile<vu32>::value&&sizeof(vu32)==4);TVec3f sdkPoint(1,0,0);PSMTXMultVec(sdkRotation,&sdkPoint,&sdkPoint);check(same(sdkPoint,TVec3f(100,201,300)));Vec sdkAliased{1,0,0};PSMTXMultVec(sdkRotation,&sdkAliased,&sdkAliased);check(sdkAliased.x==100&&sdkAliased.y==201&&sdkAliased.z==300);TVec3f objectInput(1,0,0);PSMTXMultVecSR(sdkRotation,objectInput,&objectInput);check(same(objectInput,TVec3f(0,1,0)));TQuat4f eulerQuat;eulerQuat.setEuler(.2f,-.3f,.4f);TPos3f eulerMatrix;eulerMatrix.identity();eulerQuat.makeMtx(eulerMatrix.toMtxPtr());TVec3f recovered;eulerMatrix.getEuler(recovered);check(same(recovered,TVec3f(.2f,-.3f,.4f)));eulerQuat.setEuler(0,1.57079632679f,0);eulerQuat.makeMtx(eulerMatrix.toMtxPtr());eulerMatrix.getEuler(recovered);check(same(recovered,TVec3f(0,1.57079632679f,0)));eulerQuat.setEuler(0,-1.57079632679f,0);eulerQuat.makeMtx(eulerMatrix.toMtxPtr());eulerMatrix.getEuler(recovered);check(same(recovered,TVec3f(0,-1.57079632679f,0)));TProj3f projection;projection.makePerspective(90,2,1,101);check(std::abs(projection.get(0,0)-.5f)<1e-5f&&std::abs(projection.get(1,1)-1)<1e-5f);check(std::abs((-projection.get(2,2)+projection.get(2,3))+1)<1e-5f);check(std::abs(-101*projection.get(2,2)+projection.get(2,3))<1e-5f);TVec3f crossA(1,0,0),crossB(0,1,0);PSVECCrossProduct(&crossA,&crossB,&crossA);check(same(crossA,TVec3f(0,0,1)));crossA={1,0,0};PSVECCrossProduct(&crossB,&crossA,&crossA);check(same(crossA,TVec3f(0,0,-1)));Vec sdkCrossA{1,0,0},sdkCrossB{0,1,0};PSVECCrossProduct(&sdkCrossA,&sdkCrossB,&sdkCrossB);check(sdkCrossB.x==0&&sdkCrossB.y==0&&sdkCrossB.z==1);Vec copiedScale=TVec3f(2,3,4);check(copiedScale.x==2&&copiedScale.y==3&&copiedScale.z==4);TPos3f a,b,combined; a.identity();b.identity();
    a.setXDir(0,1,0);a.setYDir(-1,0,0);a.setTrans(10,20,30);
    b.set(2,0,0,3,0,3,0,4,0,0,4,5);
    TVec3f original(1,2,3),p,q;
    b.mult(original,p);a.mult(p,q);combined.concat(a,b);combined.mult(original,p);
    check(same(p,q)&&same(p,TVec3f(0,25,47)));
    a.concat(b);a.mult(original,p);check(same(p,q));
    combined.concat(combined,b);a.concat(a,b);check(a.get(0,3)==combined.get(0,3));
    TPos3f right=b;right.concat(a,right);TPos3f expected;expected.concat(a,b);
    right.mult(original,p);expected.mult(original,q);check(same(p,q));
    TPos3f rotation;rotation.identity();rotation.setXDir(0,1,0);rotation.setYDir(-1,0,0);rotation.setTrans(10,20,30);
    p=original;rotation.mult(p,p);rotation.multTranspose(p,p);check(same(p,original));
    p=original;rotation.mult33(p);check(same(p,TVec3f(-2,1,3)));
    rotation.scaleXYZ(2);rotation.getTrans(p);check(same(p,TVec3f(10,20,30)));
    rotation.scale(3);rotation.getTrans(p);check(same(p,TVec3f(30,60,90)));
    check(std::abs(rotation.determinant33()-216)<1e-5f);
    rotation.identity33();rotation.getTrans(p);check(same(p,TVec3f(30,60,90)));
    rotation.zeroTrans();rotation.getTrans(p);check(same(p,TVec3f(0,0,0)));
    TPos3f lookAt;lookAt.setPositionFromLookAt(TVec3f(1,2,3),TVec3f(0,1,0),TVec3f(1,2,2));lookAt.mult(TVec3f(1,2,3),p);check(same(p,TVec3f(0,0,0)));lookAt.mult(TVec3f(1,2,2),p);check(same(p,TVec3f(0,0,-1)));check(std::abs(lookAt.determinant33()-1)<1e-5f);lookAt.setPositionFromLookAt(TVec3f(3,4,5),TVec3f(0,1,0),TVec3f(-2,1,0));lookAt.mult(TVec3f(3,4,5),p);check(same(p,TVec3f(0,0,0)));lookAt.mult(TVec3f(-2,1,0),p);check(std::abs(p.x)<1e-5f&&std::abs(p.y)<1e-5f&&p.z<0);check(std::abs(lookAt.determinant33()-1)<1e-5f);
    TPos3f copied(rotation.toMtxPtr());check(copied.dot()==3);
    const TVec2f twoD(3,4);check(twoD.length()==5&&twoD.squared()==25);const auto difference2D=twoD-TVec2f(1,7);check(difference2D.x==2&&difference2D.y==-3&&twoD.x==3&&twoD.y==4);TVec2f alias2D(3,4);alias2D.sub(TVec2f(1,7),alias2D);check(alias2D.x==-2&&alias2D.y==3);check(TVec2f(0,0).length()==0);const TVec2f slideInput(0,1);check((TVec2f(0,1)-slideInput).length()==0&&(TVec2f(0,-1)-slideInput).length()==2);check(std::abs((TVec2f(-1,0)-slideInput).length()-std::sqrt(2.0f))<1e-6f);
    const TVec2f bounds(2,4);const auto expanded=bounds+TVec2f(-10,10);check(expanded.x==-8&&expanded.y==14&&bounds.x==2&&bounds.y==4);TVec3f searchPoint(1,0,0);PSMTXMultVec(sdkRotation,searchPoint,&searchPoint);check(same(searchPoint,TVec3f(100,201,300)));
    TVec2f clippedFloat(7,-3);clippedFloat.zero();check(clippedFloat.x==0&&clippedFloat.y==0);TVec2s clippedInteger(7,-3);clippedInteger.zero();check(clippedInteger.x==0&&clippedInteger.y==0);
    TVec3f projected;projected.killElement2(TVec3f(2,3,4),TVec3f(0,0,1));check(same(projected,TVec3f(2,3,0)));TQuat4f zOnly;zOnly.setEulerZ(1.57079632679f);zOnly.getXDir(projected);check(same(projected,TVec3f(0,1,0)));
    TVec2f v(2.0f,4.0f);v.setMin(TVec2f(3.0f,1.0f));check(v.x==2&&v.y==1);TVec2f length2D(3,4);check(length2D.setLength(10)==5&&std::abs(length2D.x-6)<1e-5f&&std::abs(length2D.y-8)<1e-5f);const auto copied2D=length2D;check(copied2D.x==6&&copied2D.y==8);TVec3f copy3D(4,5,6);check(copy3D.copy().x==4&&copy3D.copy().y==5&&copy3D.copy().z==6);TVec3f orthogonal2(2,3,4);orthogonal2.orthogonalize2(TVec3f(0,0,1));check(same(orthogonal2,TVec3f(2,3,0)));TQuat4f axisQuat;axisQuat.setRotate(0,0,1,1.57079632679f);axisQuat.getXDir(projected);check(same(projected,TVec3f(0,1,0)));TQuat4f sdkMultiply;PSQUATMultiply(&axisQuat,&axisQuat,&sdkMultiply);TQuat4f doubleAxis;doubleAxis.setEulerZ(3.14159265359f);check(std::abs(sdkMultiply.dot(doubleAxis)-1)<1e-5f);TPos3f rt;rt.setRT(TVec3f(0,0,1.57079632679f),TVec3f(7,8,9));rt.mult(TVec3f(1,0,0),projected);check(same(projected,TVec3f(7,9,9)));
    v.setMax(TVec2f(3.0f,0.0f));check(v.x==3&&v.y==1&&v.isAbove(TVec2f(2.0f,1.0f)));
    TPos3f transposedAxes;transposedAxes.setXYZDir2(TVec3f(1,2,3),TVec3f(4,5,6),TVec3f(7,8,9));check(transposedAxes.get(0,0)==1&&transposedAxes.get(0,1)==2&&transposedAxes.get(0,2)==3&&transposedAxes.get(1,0)==4&&transposedAxes.get(2,2)==9);
    TPos3f normalizedBasis;normalizedBasis.set(2,0,0,7,0,3,0,8,0,0,4,9);normalizedBasis.normalizeBasis();check(std::abs(normalizedBasis.get(0,0)-1)<1e-5f&&std::abs(normalizedBasis.get(1,1)-1)<1e-5f&&std::abs(normalizedBasis.get(2,2)-1)<1e-5f&&normalizedBasis.get(0,3)==7&&normalizedBasis.get(1,3)==8&&normalizedBasis.get(2,3)==9);TPos3f limitedTurn;limitedTurn.makeRotate(TVec3f(1,0,0),TVec3f(0,1,0),0.25f);TVec3f turnDirection;limitedTurn.getXDir(turnDirection);check(std::abs(turnDirection.x-std::cos(0.25f))<1e-5f&&std::abs(turnDirection.y-std::sin(0.25f))<1e-5f);TVec3f hermite;hermite.cubic(TVec3f(0,0,0),TVec3f(0,1,0),TVec3f(0,1,0),TVec3f(0,2,0),0.5f);check(same(hermite,TVec3f(0,1,0)));
    std::cout<<"PASS: native affine matrix composition, aliased operands, rotation/translation and scale semantics\n";
}


















