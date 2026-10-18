#include <native_no_weak.h>
#include <revolution/mtx.h>
#include "Native/GDCompat.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>
static void equal(const Mtx a,const Mtx b){for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(std::fabs(a[r][c]-b[r][c])<.00001f);}
int main(){
    assert(__cntlzw(0)==32);for(unsigned i=0;i<32;++i)assert(__cntlzw(u32(1)<<i)==31-i);assert(__cntlzw(0xffffffff)==0);
    Mtx a={{2,1,0,7},{0,3,1,-5},{1,0,-4,9}},b={{0,-1,0,3},{1,0,0,4},{0,0,2,-6}},combined,left,right;
    PSMTXConcat(a,b,combined);
    Vec input{3,-2,6},first,expected,actual;
    PSMTXMultVec(b,&input,&first);PSMTXMultVec(a,&first,&expected);PSMTXMultVec(combined,&input,&actual);
    assert(std::fabs(expected.x-actual.x)<.00001f&&std::fabs(expected.y-actual.y)<.00001f&&std::fabs(expected.z-actual.z)<.00001f);
    std::memcpy(left,a,sizeof(a));PSMTXConcat(left,b,left);equal(left,combined);
    std::memcpy(right,b,sizeof(b));PSMTXConcat(a,right,right);equal(right,combined);
    PSMTXConcat(a,a,combined);std::memcpy(left,a,sizeof(a));PSMTXConcat(left,left,left);equal(left,combined);
    Mtx translation;PSMTXTrans(translation,7,-5,9);PSMTXMultVec(translation,&input,&actual);assert(actual.x==10&&actual.y==-7&&actual.z==15);
    Mtx rotation;Vec rotationInput;
    rotationInput={0,1,0};PSMTXRotRad(rotation,'x',1.5707963267948966f);PSMTXMultVec(rotation,&rotationInput,&actual);assert(std::fabs(actual.x)<1e-6f&&std::fabs(actual.y)<1e-6f&&std::fabs(actual.z-1)<1e-6f);
    rotationInput={0,0,1};PSMTXRotRad(rotation,'Y',1.5707963267948966f);PSMTXMultVec(rotation,&rotationInput,&actual);assert(std::fabs(actual.x-1)<1e-6f&&std::fabs(actual.y)<1e-6f&&std::fabs(actual.z)<1e-6f);
    rotationInput={1,0,0};PSMTXRotRad(rotation,'z',1.5707963267948966f);PSMTXMultVec(rotation,&rotationInput,&actual);assert(std::fabs(actual.x)<1e-6f&&std::fabs(actual.y-1)<1e-6f&&std::fabs(actual.z)<1e-6f);
    Mtx unchanged;for(int r=0;r<3;++r)for(int c=0;c<4;++c)unchanged[r][c]=float(r*4+c+1);Mtx invalid;std::memcpy(invalid,unchanged,sizeof(Mtx));PSMTXRotRad(invalid,'q',0.5f);equal(invalid,unchanged);
    PSMTXMultVecSR(translation,&input,&actual);assert(actual.x==input.x&&actual.y==input.y&&actual.z==input.z);
    Vec direction{3,-4,12},normalized;PSVECNormalize(&direction,&normalized);
    assert(std::fabs(normalized.x-3.0f/13)<.00001f&&std::fabs(normalized.y+4.0f/13)<.00001f&&std::fabs(normalized.z-12.0f/13)<.00001f);
    PSVECNormalize(&direction,&direction);assert(direction.x==normalized.x&&direction.y==normalized.y&&direction.z==normalized.z);
    Vec zero{0,0,0};PSVECNormalize(&zero,&normalized);assert(std::isnan(normalized.x)&&std::isnan(normalized.y)&&std::isnan(normalized.z));
    Vec magnitude{3,4,12};assert(PSVECMag(&magnitude)==13.0f);magnitude={0,0,0};assert(PSVECMag(&magnitude)==0.0f);
    Quaternion qa{0.2f,-0.3f,0.4f,0.5f},qb{-0.7f,0.1f,0.6f,0.8f},qp;
    PSQUATMultiply(&qa,&qb,&qp);
    const Quaternion expectedProduct{qa.w*qb.x+qa.x*qb.w+qa.y*qb.z-qa.z*qb.y,
        qa.w*qb.y-qa.x*qb.z+qa.y*qb.w+qa.z*qb.x,
        qa.w*qb.z+qa.x*qb.y-qa.y*qb.x+qa.z*qb.w,
        qa.w*qb.w-qa.x*qb.x-qa.y*qb.y-qa.z*qb.z};
    assert(std::memcmp(&qp,&expectedProduct,sizeof(qp))==0);
    Quaternion aliasA=qa,aliasB=qb;PSQUATMultiply(&aliasA,&aliasB,&aliasA);assert(std::memcmp(&aliasA,&expectedProduct,sizeof(aliasA))==0);
    aliasA=qa;aliasB=qb;PSQUATMultiply(&aliasA,&aliasB,&aliasB);assert(std::memcmp(&aliasB,&expectedProduct,sizeof(aliasB))==0);
Mtx identity,scaled,copied,translated,scaleApplied;PSMTXIdentity(identity);for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(identity[r][c]==(r==c?1.0f:0.0f));PSMTXScale(scaled,-2,3,0.5f);PSMTXMultVec(scaled,&input,&actual);assert(actual.x==-6 && actual.y==-6 && actual.z==3);PSMTXCopy(a,copied);equal(a,copied);PSMTXCopy(copied,copied);equal(a,copied);PSMTXScaleApply(a,scaleApplied,-2,3,0.5f);for(int c=0;c<4;++c){assert(scaleApplied[0][c]==a[0][c]*-2);assert(scaleApplied[1][c]==a[1][c]*3);assert(scaleApplied[2][c]==a[2][c]*0.5f);}PSMTXScaleApply(copied,copied,-2,3,0.5f);equal(copied,scaleApplied);PSMTXCopy(a,copied);PSMTXTransApply(a,translated,2,-3,4);for(int r=0;r<3;++r)for(int c=0;c<3;++c)assert(translated[r][c]==a[r][c]);assert(translated[0][3]==9 && translated[1][3]==-8 && translated[2][3]==13);PSMTXTransApply(copied,copied,2,-3,4);equal(copied,translated);
Mtx44 full,fullCopy;PSMTX44Identity(full);for(int r=0;r<4;++r)for(int c=0;c<4;++c)assert(full[r][c]==(r==c?1.0f:0.0f));full[3][1]=7;PSMTX44Copy(full,fullCopy);assert(!std::memcmp(full,fullCopy,sizeof(full)));PSMTX44Copy(fullCopy,fullCopy);assert(!std::memcmp(full,fullCopy,sizeof(full)));
std::puts("PASS native UI matrix identity, copy, scale apply, translation apply and destination aliases");
Mtx light; C_MTXLightOrtho(light,3,-1,-2,6,.5f,.5f,.5f,.5f);
Vec lower{-2,-1,-9},upper{6,3,-90};PSMTXMultVec(light,&lower,&actual);assert(std::fabs(actual.x)<1e-6f&&std::fabs(actual.y)<1e-6f&&actual.z==1);PSMTXMultVec(light,&upper,&actual);assert(std::fabs(actual.x-1)<1e-6f&&std::fabs(actual.y-1)<1e-6f&&actual.z==1);
C_MTXLightPerspective(light,90,2,.5f,.5f,.5f,.5f);
Vec edge{4,2,-2};PSMTXMultVec(light,&edge,&actual);assert(std::fabs(actual.x/actual.z-1)<1e-6f&&std::fabs(actual.y/actual.z-1)<1e-6f);edge={-4,-2,-2};PSMTXMultVec(light,&edge,&actual);assert(std::fabs(actual.x/actual.z)<1e-6f&&std::fabs(actual.y/actual.z)<1e-6f);
std::puts("PASS light projection: asymmetric orthographic corners and perspective frustum edges");
    std::puts("PASS SDK affine composition order, all destination aliases and translation");
    std::puts("PASS SDK axis rotations, case normalization and invalid-axis preservation");
}


