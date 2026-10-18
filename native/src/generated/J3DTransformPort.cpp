#include "JSystem/J3DGraphBase/J3DTransform.hpp"
#include "JSystem/J3DGraphBase/J3DStruct.hpp"
#include "JSystem/JMath.hpp"
#include "JSystem/JMath/JMATrigonometric.hpp"
#include <cmath>
#include <revolution/mtx.h>

const J3DTransformInfo j3dDefaultTransformInfo = {
    {1.0f, 1.0f, 1.0f},
    {0, 0, 0},
    0,
    {0.0f, 0.0f, 0.0f},
};

const Vec j3dDefaultScale = {1.0f, 1.0f, 1.0f};

const Mtx j3dDefaultMtx = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
};

void J3DCalcBBoardMtx(Mtx mtx) {
    f32 x = (mtx[0][0] * mtx[0][0]) + (mtx[1][0] * mtx[1][0]) + (mtx[2][0] * mtx[2][0]);
    f32 y = (mtx[0][1] * mtx[0][1]) + (mtx[1][1] * mtx[1][1]) + (mtx[2][1] * mtx[2][1]);
    f32 z = (mtx[0][2] * mtx[0][2]) + (mtx[1][2] * mtx[1][2]) + (mtx[2][2] * mtx[2][2]);

    if (x > 0.0f) {
        x = sqrt(x);
    }

    if (y > 0.0f) {
        y = sqrt(y);
    }

    if (z > 0.0f) {
        z = sqrt(z);
    }

    mtx[0][0] = x;
    mtx[0][1] = 0.0f;
    mtx[0][2] = 0.0f;
    mtx[2][0] = 0.0f;
    mtx[2][1] = 0.0f;
    mtx[1][0] = 0.0f;
    mtx[1][1] = y;
    mtx[1][2] = 0.0f;
    mtx[2][2] = z;
}

void J3DCalcYBBoardMtx(Mtx mtx) {
    f32 x = (mtx[0][0] * mtx[0][0]) + (mtx[1][0] * mtx[1][0]) + (mtx[2][0] * mtx[2][0]);
    f32 z = (mtx[0][2] * mtx[0][2]) + (mtx[1][2] * mtx[1][2]) + (mtx[2][2] * mtx[2][2]);

    if (x > 0.0f) {
        x = std::sqrt(x);
    }

    if (z > 0.0f) {
        z = std::sqrt(z);
    }

    Vec vec = {0.0f, -mtx[2][1], mtx[1][1]};
    const f32 inverseLength=1.0f/std::sqrt(vec.x*vec.x+vec.y*vec.y+vec.z*vec.z); vec.x*=inverseLength; vec.y*=inverseLength; vec.z*=inverseLength;

    mtx[0][0] = x;
    mtx[0][2] = 0.0f;
    mtx[1][0] = 0.0f;

    mtx[1][2] = vec.y * z;
    mtx[2][0] = 0.0f;
    mtx[2][2] = vec.z * z;
}

void J3DPSCalcInverseTranspose(Mtx src,Mtx33 dst) {
    Mtx inverse;
    if(!PSMTXInverse(src,inverse))return;
    for(int r=0;r<3;++r)for(int c=0;c<3;++c)dst[r][c]=inverse[c][r];
}
void J3DPSMtxArrayConcat(Mtx left,Mtx input,Mtx output,u32 count) {
    Mtx stableLeft;
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)stableLeft[r][c]=left[r][c];
    for(u32 i=0;i<count;++i)PSMTXConcat(stableLeft,input+3*i,output+3*i);
}
void J3DScaleNrmMtx(Mtx matrix,const Vec& scale) {
    const f32 x=scale.x,y=scale.y,z=scale.z;
    for(int r=0;r<3;++r){matrix[r][0]*=x;matrix[r][1]*=y;matrix[r][2]*=z;}
}
void J3DScaleNrmMtx33(Mtx33 matrix,const Vec& scale) {
    const f32 x=scale.x,y=scale.y,z=scale.z;
    for(int r=0;r<3;++r){matrix[r][0]*=x;matrix[r][1]*=y;matrix[r][2]*=z;}
}
void J3DMtxProjConcat(Mtx left,Mtx right,Mtx dst) {
    // Original assembly reads four rows from the right projection matrix,
    // despite the SDK's Mtx parameter spelling (a decayed pointer).
    Mtx result;
    for(int r=0;r<3;++r)for(int c=0;c<4;++c){
        f32 value=left[r][0]*right[0][c];
        for(int k=1;k<4;++k)value+=left[r][k]*right[k][c];
        result[r][c]=value;
    }
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)dst[r][c]=result[r][c];
}
void J3DGetTranslateRotateMtx(const J3DTransformInfo& tx, Mtx dst) {
    f32 cxsz;
    f32 sxcz;

    f32 sx = JMASSin(tx.mRotation.x), cx = JMath::sSinCosTable.cosShort(tx.mRotation.x);
    f32 sy = JMASSin(tx.mRotation.y), cy = JMath::sSinCosTable.cosShort(tx.mRotation.y);
    f32 sz = JMASSin(tx.mRotation.z), cz = JMath::sSinCosTable.cosShort(tx.mRotation.z);

    dst[2][0] = -sy;
    dst[0][0] = cz * cy;
    dst[1][0] = sz * cy;
    dst[2][1] = cy * sx;
    dst[2][2] = cy * cx;

    cxsz = cx * sz;
    sxcz = sx * cz;
    dst[0][1] = sxcz * sy - cxsz;
    dst[1][2] = cxsz * sy - sxcz;

    cxsz = sx * sz;
    sxcz = cx * cz;
    dst[0][2] = sxcz * sy + cxsz;
    dst[1][1] = cxsz * sy + sxcz;

    dst[0][3] = tx.mTranslate.x;
    dst[1][3] = tx.mTranslate.y;
    dst[2][3] = tx.mTranslate.z;
}

void J3DGetTranslateRotateMtx(s16 rx, s16 ry, s16 rz, f32 tx, f32 ty, f32 tz, Mtx dst) {
    f32 cxsz;
    f32 sxcz;

    f32 sx = JMASSin(rx), cx = JMath::sSinCosTable.cosShort(rx);
    f32 sy = JMASSin(ry), cy = JMath::sSinCosTable.cosShort(ry);
    f32 sz = JMASSin(rz), cz = JMath::sSinCosTable.cosShort(rz);

    dst[2][0] = -sy;
    dst[0][0] = cz * cy;
    dst[1][0] = sz * cy;
    dst[2][1] = cy * sx;
    dst[2][2] = cy * cx;

    cxsz = cx * sz;
    sxcz = sx * cz;
    dst[0][1] = sxcz * sy - cxsz;
    dst[1][2] = cxsz * sy - sxcz;

    cxsz = sx * sz;
    sxcz = cx * cz;
    dst[0][2] = sxcz * sy + cxsz;
    dst[1][1] = cxsz * sy + sxcz;

    dst[0][3] = tx;
    dst[1][3] = ty;
    dst[2][3] = tz;
}

void J3DGetTextureMtx(const J3DTextureSRTInfo& srt, const Vec& center, Mtx dst) {
    f32 sr = JMASSin(srt.mRotation), cr = JMath::sSinCosTable.cosShort(srt.mRotation);

    f32 cx = srt.mScaleX * cr;
    f32 sx = srt.mScaleX * sr;
    f32 sy = srt.mScaleY * sr;
    f32 cy = srt.mScaleY * cr;

    dst[0][0] = cx;
    dst[0][1] = -sx;
    dst[0][2] = (-cx * center.x + sx * center.y) + center.x + srt.mTranslationX;

    dst[1][0] = sy;
    dst[1][1] = cy;
    dst[1][2] = (-sy * center.x - cy * center.y) + center.y + srt.mTranslationY;

    dst[0][3] = dst[1][3] = dst[2][0] = dst[2][1] = dst[2][3] = 0.0f;
    dst[2][2] = 1.0f;
}

void J3DGetTextureMtxOld(const J3DTextureSRTInfo& srt, const Vec& center, Mtx dst) {
    f32 sr = JMASSin(srt.mRotation), cr = JMath::sSinCosTable.cosShort(srt.mRotation);

    f32 cx = srt.mScaleX * cr;
    f32 sx = srt.mScaleX * sr;
    f32 sy = srt.mScaleY * sr;
    f32 cy = srt.mScaleY * cr;

    dst[0][0] = cx;
    dst[0][1] = -sx;
    dst[0][3] = (-cx * center.x + sx * center.y) + center.x + srt.mTranslationX;

    dst[1][0] = sy;
    dst[1][1] = cy;
    dst[1][3] = (-sy * center.x - cy * center.y) + center.y + srt.mTranslationY;

    dst[0][2] = dst[1][2] = dst[2][0] = dst[2][1] = dst[2][3] = 0.0f;
    dst[2][2] = 1.0f;
}

void J3DGetTextureMtxMaya(const J3DTextureSRTInfo& srt, Mtx dst) {
    f32 sr = JMASSin(srt.mRotation), cr = JMath::sSinCosTable.cosShort(srt.mRotation);
    f32 tx = srt.mTranslationX - 0.5f;
    f32 ty = srt.mTranslationY - 0.5f;

    dst[0][0] = srt.mScaleX * cr;
    dst[0][1] = srt.mScaleY * sr;
    dst[0][2] = tx * cr - sr * (ty + srt.mScaleY) + 0.5f;

    dst[1][0] = -srt.mScaleX * sr;
    dst[1][1] = srt.mScaleY * cr;
    dst[1][2] = -tx * sr - cr * (ty + srt.mScaleY) + 0.5f;

    dst[0][3] = dst[1][3] = dst[2][0] = dst[2][1] = dst[2][3] = 0.0f;
    dst[2][2] = 1.0f;
}

void J3DGetTextureMtxMayaOld(const J3DTextureSRTInfo& srt, Mtx dst) {
    f32 sr = JMASSin(srt.mRotation), cr = JMath::sSinCosTable.cosShort(srt.mRotation);
    f32 tx = srt.mTranslationX - 0.5f;
    f32 ty = srt.mTranslationY - 0.5f;

    dst[0][0] = srt.mScaleX * cr;
    dst[0][1] = srt.mScaleY * sr;
    dst[0][3] = tx * cr - sr * (ty + srt.mScaleY) + 0.5f;

    dst[1][0] = -srt.mScaleX * sr;
    dst[1][1] = srt.mScaleY * cr;
    dst[1][3] = -tx * sr - cr * (ty + srt.mScaleY) + 0.5f;

    dst[0][2] = dst[1][2] = dst[2][0] = dst[2][1] = dst[2][3] = 0.0f;
    dst[2][2] = 1.0f;
}

