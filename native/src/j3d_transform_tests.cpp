#include <JSystem/J3DGraphBase/J3DTransform.hpp>
#include <JSystem/J3DGraphBase/J3DStruct.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
    Mtx affine={{2,1,0,7},{0,3,1,-5},{1,0,-4,9}};Mtx33 normal;
    J3DPSCalcInverseTranspose(affine,normal);
    Vec transformedTangent;Vec tangent{1,0,0};PSMTXMultVecSR(affine,&tangent,&transformedTangent);
    Vec transformedNormal{normal[0][1],normal[1][1],normal[2][1]};
    assert(std::fabs(transformedTangent.x*transformedNormal.x+transformedTangent.y*transformedNormal.y+transformedTangent.z*transformedNormal.z)<.00001f);
    Mtx inverse;assert(PSMTXInverse(affine,inverse));for(int r=0;r<3;++r)for(int c=0;c<3;++c)assert(normal[r][c]==inverse[c][r]);
    Mtx singular={{0,0,0,0},{0,0,0,0},{0,0,0,0}};J3DPSCalcInverseTranspose(singular,normal);for(int r=0;r<3;++r)for(int c=0;c<3;++c)assert(normal[r][c]==inverse[c][r]);
    J3DCalcBBoardMtx(affine);assert(std::fabs(affine[0][0]-std::sqrt(5.0f))<.00001f);assert(std::fabs(affine[1][1]-std::sqrt(10.0f))<.00001f);assert(std::fabs(affine[2][2]-std::sqrt(17.0f))<.00001f);assert(affine[0][3]==7&&affine[1][3]==-5&&affine[2][3]==9);
    Mtx y={{2,0,0,7},{0,3,0,-5},{0,4,6,9}};J3DCalcYBBoardMtx(y);
    assert(y[0][0]==2&&y[0][1]==0&&y[1][1]==3&&y[2][1]==4);
    assert(std::fabs(y[1][2]+4.8f)<.00001f&&std::fabs(y[2][2]-3.6f)<.00001f);
    assert(y[0][3]==7&&y[1][3]==-5&&y[2][3]==9);
    Mtx inputs[2]={{{2,1,0,7},{0,3,1,-5},{1,0,-4,9}},{{0,-1,0,3},{1,0,0,4},{0,0,2,-6}}},outputs[2],reference[2];
    PSMTXConcat(y,inputs[0],reference[0]);PSMTXConcat(y,inputs[1],reference[1]);
    J3DPSMtxArrayConcat(y,inputs[0],outputs[0],2);
    for(int i=0;i<2;++i)for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(outputs[i][r][c]==reference[i][r][c]);
    J3DPSMtxArrayConcat(y,inputs[0],inputs[0],2);
    for(int i=0;i<2;++i)for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(inputs[i][r][c]==reference[i][r][c]);
    J3DPSMtxArrayConcat(y,inputs[0],outputs[0],0);
    for(int i=0;i<2;++i)for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(outputs[i][r][c]==reference[i][r][c]);
    Mtx scaled={{1,2,3,7},{4,5,6,-5},{7,8,9,9}};Mtx33 scaled33={{1,2,3},{4,5,6},{7,8,9}};Vec scale{2,-3,.5f};
    J3DScaleNrmMtx(scaled,scale);J3DScaleNrmMtx33(scaled33,scale);
    for(int r=0;r<3;++r){assert(scaled[r][0]==(3*r+1)*2);assert(scaled[r][1]==(3*r+2)*-3);assert(scaled[r][2]==(3*r+3)*.5f);for(int c=0;c<3;++c)assert(scaled[r][c]==scaled33[r][c]);}
    assert(scaled[0][3]==7&&scaled[1][3]==-5&&scaled[2][3]==9);
    Mtx left={{2,1,0,7},{0,3,1,-5},{1,0,-4,9}};
    Mtx44 projection={{1,0,2,0},{0,3,0,1},{0,0,-2,-3},{0,0,-1,0}};
    Mtx product;
    J3DMtxProjConcat(left,projection,product);
    const float point[4]={2,-3,5,1};
    float projected[4]={};
    for(int r=0;r<4;++r)for(int c=0;c<4;++c)projected[r]+=projection[r][c]*point[c];
    for(int r=0;r<3;++r){
        float expected=0,actual=0;
        for(int c=0;c<4;++c){expected+=left[r][c]*projected[c];actual+=product[r][c]*point[c];}
        assert(actual==expected);
    }
    J3DMtxProjConcat(left,projection,left);
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(left[r][c]==product[r][c]);
    Mtx originalLeft={{2,1,0,7},{0,3,1,-5},{1,0,-4,9}};
    J3DMtxProjConcat(originalLeft,projection,projection);
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)assert(projection[r][c]==product[r][c]);
    assert(projection[3][2]==-1 && projection[3][3]==0);
    J3DTextureSRTInfo srt;
    srt.mScaleX=2;srt.mScaleY=-3;srt.mRotation=0;
    srt.mTranslationX=.25f;srt.mTranslationY=-.125f;
    Vec center{.2f,.7f,0};Mtx tex,oldTex;
    J3DGetTextureMtx(srt,center,tex);
    J3DGetTextureMtxOld(srt,center,oldTex);
    assert(std::fabs(tex[0][2]-(.25f-.2f))<.00001f);
    assert(std::fabs(tex[1][2]-(-.125f+4*.7f))<.00001f);
    for(int r=0;r<2;++r){assert(tex[r][2]==oldTex[r][3]);assert(tex[r][3]==0&&oldTex[r][2]==0);}
    srt.mRotation=16384;
    J3DGetTextureMtx(srt,center,tex);
    const float u=.6f,v=-.2f;
    assert(std::fabs((tex[0][0]*u+tex[0][1]*v+tex[0][2])-(-2*(v-center.y)+center.x+srt.mTranslationX))<.00001f);
    assert(std::fabs((tex[1][0]*u+tex[1][1]*v+tex[1][2])-(-3*(u-center.x)+center.y+srt.mTranslationY))<.00001f);
    J3DGetTextureMtxMaya(srt,tex);J3DGetTextureMtxMayaOld(srt,oldTex);
    assert(std::fabs(tex[0][0])<.00001f&&std::fabs(tex[0][1]+3)<.00001f);
    assert(std::fabs(tex[1][0]+2)<.00001f&&std::fabs(tex[1][1])<.00001f);
    for(int r=0;r<2;++r){assert(tex[r][2]==oldTex[r][3]);assert(tex[r][3]==0&&oldTex[r][2]==0);}
    Mtx rotated;J3DGetTranslateRotateMtx(0,0,16384,7,-5,9,rotated);
    assert(std::fabs(rotated[0][0])<.00001f&&std::fabs(rotated[0][1]+1)<.00001f);
    assert(std::fabs(rotated[1][0]-1)<.00001f&&rotated[2][2]==1);
    assert(rotated[0][3]==7&&rotated[1][3]==-5&&rotated[2][3]==9);
    std::puts("PASS original texture/rotation matrices and native normal/projection transforms");
}
