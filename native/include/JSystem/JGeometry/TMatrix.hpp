#pragma once
#include <revolution/types.h>
#include <algorithm>
#include <revolution/mtx.h>
#include "JSystem/JGeometry/TVec.hpp"
inline f32 SIN(f32 angle){return std::sin(angle);}
inline f32 COS(f32 angle){return std::cos(angle);}
// Native scalar implementation of the pinned JGeometry affine matrix core.
// No console ABI overlay or paired-single rounding equivalence is implied.
namespace JGeometry {
template<class T> struct SMatrix34C {
    using ArrType=T[4];
    T mMtx[3][4];
    void set(const ArrType* src){for(int r=0;r<3;++r)for(int c=0;c<4;++c)mMtx[r][c]=src[r][c];}
    void set(const SMatrix34C& src){set(src.mMtx);}
    void set(T xx,T xy,T xz,T tx,T yx,T yy,T yz,T ty,T zx,T zy,T zz,T tz){
        mMtx[0][0]=xx;mMtx[0][1]=xy;mMtx[0][2]=xz;mMtx[0][3]=tx;
        mMtx[1][0]=yx;mMtx[1][1]=yy;mMtx[1][2]=yz;mMtx[1][3]=ty;
        mMtx[2][0]=zx;mMtx[2][1]=zy;mMtx[2][2]=zz;mMtx[2][3]=tz;
    }
    void setInline(const SMatrix34C& src){set(src);}
    void setInline(const SMatrix34C* src){set(*src);}
    void setInline(const ArrType* src){set(src);}
    void scale(T value){for(int r=0;r<3;++r)for(int c=0;c<3;++c)mMtx[r][c]*=value;}
    void scaleInline(T value){scale(value);}
    T get(int r,int c)const{return mMtx[r][c];}
    T operator()(int r,int c)const{return get(r,c);}
    operator ArrType*(){return mMtx;}
    operator const ArrType*()const{return mMtx;}
    Mtx* toMtx(){static_assert(sizeof(T)==sizeof(f32));return reinterpret_cast<Mtx*>(mMtx);}
    const Mtx* toCMtx()const{static_assert(sizeof(T)==sizeof(f32));return reinterpret_cast<const Mtx*>(mMtx);}
    MtxPtr toMtxPtr(){static_assert(sizeof(T)==sizeof(f32));return reinterpret_cast<MtxPtr>(mMtx);}
    T dot(int r,int c)const{return mMtx[r][c]*mMtx[r][c];}
    T dotX()const{return dot(1,0)+dot(0,0)+dot(2,0);}
    T dotY()const{return dot(1,1)+dot(0,1)+dot(2,1);}
    T dotZ()const{return dot(1,2)+dot(0,2)+dot(2,2);}
    T dot()const{return dotX()+dotY()+dotZ();}
};
template<class T> struct TMatrix34:public T {
    void identity(){for(int r=0;r<3;++r)for(int c=0;c<4;++c)this->mMtx[r][c]=(r==c)?1.0f:0.0f;}
    void concat(const T& a,const T& b){
        T result;
        for(int r=0;r<3;++r){for(int c=0;c<3;++c)result.mMtx[r][c]=a(r,2)*b(2,c)+(a(r,0)*b(0,c)+a(r,1)*b(1,c));
        result.mMtx[r][3]=a(r,3)+(a(r,2)*b(2,3)+(a(r,0)*b(0,3)+a(r,1)*b(1,3)));}
        this->set(result);
    }
    void concat(const T& rhs){concat(*this,rhs);}
    f32 determinant33()const{return (*this)(0,0)*((*this)(1,1)*(*this)(2,2)-(*this)(1,2)*(*this)(2,1))-(*this)(0,1)*((*this)(1,0)*(*this)(2,2)-(*this)(1,2)*(*this)(2,0))+(*this)(0,2)*((*this)(1,0)*(*this)(2,1)-(*this)(1,1)*(*this)(2,0));}
    bool invert(const TMatrix34& src){
        const f32 determinant=src.determinant33();
        const f32 epsilon=32.0f*std::numeric_limits<f32>::epsilon();
        if(determinant<=epsilon&&determinant>=-epsilon)return false;
        const f32 scale=1.0f/determinant;
        T result;
        result.mMtx[0][0]=scale*(src(1,1)*src(2,2)-src(1,2)*src(2,1));
        result.mMtx[0][1]=scale*-(src(0,1)*src(2,2)-src(0,2)*src(2,1));
        result.mMtx[0][2]=scale*(src(0,1)*src(1,2)-src(0,2)*src(1,1));
        result.mMtx[1][0]=scale*-(src(1,0)*src(2,2)-src(1,2)*src(2,0));
        result.mMtx[1][1]=scale*(src(0,0)*src(2,2)-src(0,2)*src(2,0));
        result.mMtx[1][2]=scale*-(src(0,0)*src(1,2)-src(0,2)*src(1,0));
        result.mMtx[2][0]=scale*(src(1,0)*src(2,1)-src(1,1)*src(2,0));
        result.mMtx[2][1]=scale*-(src(0,0)*src(2,1)-src(0,1)*src(2,0));
        result.mMtx[2][2]=scale*(src(0,0)*src(1,1)-src(0,1)*src(1,0));
        for(int r=0;r<3;++r)result.mMtx[r][3]=-src(0,3)*result.mMtx[r][0]-src(1,3)*result.mMtx[r][1]-src(2,3)*result.mMtx[r][2];
        this->set(result);return true;
    }
    void mult(const TVec3f& src,TVec3f& dst)const{
        const auto x=src.x,y=src.y,z=src.z;
        dst.set(z*(*this)(0,2)+(x*(*this)(0,0)+y*(*this)(0,1))+(*this)(0,3),z*(*this)(1,2)+(x*(*this)(1,0)+y*(*this)(1,1))+(*this)(1,3),z*(*this)(2,2)+(x*(*this)(2,0)+y*(*this)(2,1))+(*this)(2,3));
    }
    void multTranspose(const TVec3f& src,TVec3f& dst)const{
        const auto x=src.x-(*this)(0,3),y=src.y-(*this)(1,3),z=src.z-(*this)(2,3);
        dst.set(z*(*this)(2,0)+(x*(*this)(0,0)+y*(*this)(1,0)),z*(*this)(2,1)+(x*(*this)(0,1)+y*(*this)(1,1)),z*(*this)(2,2)+(x*(*this)(0,2)+y*(*this)(1,2)));
    }
    void scale(f32 value){for(int r=0;r<3;++r)for(int c=0;c<4;++c)this->mMtx[r][c]*=value;}
    void scaleXYZ(f32 value){T::scale(value);}
};
template<class T> struct TRotation3:public T {
    void setRotateDegree(const TVec3f& rotation){setRotate(rotation*(3.14159265358979323846f/180.0f));}
        void setEulerX(f32 angle) {
            f32 s = std::sin(angle);
            f32 c = std::cos(angle);

            this->mMtx[0][0] = 1.0f;
            this->mMtx[2][1] = s;
            this->mMtx[1][1] = c;
            this->mMtx[1][2] = -s;
            this->mMtx[2][2] = c;
            this->mMtx[2][0] = 0.0f;
            this->mMtx[0][2] = 0.0f;
            this->mMtx[1][0] = 0.0f;
            this->mMtx[0][1] = 0.0f;
        }

        void setEulerY(f32 angle) {
            f32 s = std::sin(angle);
            f32 c = std::cos(angle);

            this->mMtx[0][2] = s;
            this->mMtx[1][1] = 1.0f;
            this->mMtx[0][0] = c;
            this->mMtx[2][0] = -s;
            this->mMtx[2][2] = c;
            this->mMtx[2][1] = 0.0f;
            this->mMtx[1][2] = 0.0f;
            this->mMtx[1][0] = 0.0f;
            this->mMtx[0][1] = 0.0f;
        }

    void setEulerZ(f32 angle){
        const f32 s=std::sin(angle),c=std::cos(angle);
        this->mMtx[1][0]=s;this->mMtx[0][0]=c;this->mMtx[0][1]=-s;this->mMtx[1][1]=c;
        this->mMtx[2][2]=1;this->mMtx[2][1]=0;this->mMtx[1][2]=0;this->mMtx[2][0]=0;this->mMtx[0][2]=0;
    }
        void setRotate(const TVec3f& rAxis, f32 angle) {
            TVec3f vec;
            vec.normalize(rAxis);

            f32 s = std::sin(angle);
            f32 c = std::cos(angle);

            f32 negc = 1.0f - c;

            f32 x, y, z;

            x = vec.x;
            y = vec.y;
            z = vec.z;

            f32 xx, yy, zz;
            xx = x * x;
            yy = y * y;
            zz = z * z;

            this->mMtx[0][0] = c + negc * xx;
            this->mMtx[0][1] = negc * x * y - s * z;
            this->mMtx[0][2] = negc * x * z + s * y;
            this->mMtx[1][0] = negc * x * y + s * z;
            this->mMtx[1][1] = c + negc * yy;
            this->mMtx[1][2] = negc * y * z - s * x;
            this->mMtx[2][0] = negc * x * z - s * y;
            this->mMtx[2][1] = negc * y * z + s * x;
            this->mMtx[2][2] = c + negc * zz;
        }

    void setRotate(const TVec3f& from,const TVec3f& to){TQuat4<float> q;q.setRotate(from,to);setQuat(q);}
    void setRotate(const TVec3f& rotation){setRotate(rotation.x,rotation.y,rotation.z);}
    void setRotate(f32 rx,f32 ry,f32 rz){
        const f32 cosZ=std::cos(rz),cosY=std::cos(ry),cosX=std::cos(rx);
        const f32 sinZ=std::sin(rz),sinY=std::sin(ry),sinX=std::sin(rx);
        const f32 sXsY=sinX*sinY,cXcZ=cosX*cosZ,cXsZ=cosX*sinZ;
        this->mMtx[0][0]=cosY*cosZ;this->mMtx[1][0]=cosY*sinZ;this->mMtx[2][0]=-sinY;
        this->mMtx[0][1]=sXsY*cosZ-cosX*sinZ;this->mMtx[1][1]=cosX*cosZ+sXsY*sinZ;this->mMtx[2][1]=sinX*cosY;
        this->mMtx[0][2]=cXcZ*sinY+sinX*sinZ;this->mMtx[1][2]=cXsZ*sinY-sinX*cosZ;this->mMtx[2][2]=cosX*cosY;
    }
        void getQuat(TQuat4<float>& rDest) const {
            if ((*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2) >= 0.0f) {
                f32 root = (*this)(0, 0) + (*this)(1, 1) + (*this)(2, 2);
                root += 1.0f;
                root = std::sqrt(root);
                f32 scale = 0.5f / root;
                rDest.w = 0.5f * root;
                rDest.x = scale * ((*this)(2, 1) - (*this)(1, 2));
                rDest.y = scale * ((*this)(0, 2) - (*this)(2, 0));
                rDest.z = scale * ((*this)(1, 0) - (*this)(0, 1));
            } else {
                f32 maximum = std::max(((*this)(0, 0) >= (*this)(1, 1) ? (*this)(0, 0) : (*this)(1, 1)), (*this)(2, 2));

                if (maximum == (*this)(0, 0)) {
                    f32 root = std::sqrt(1.0f + ((*this)(0, 0) - ((*this)(1, 1) + (*this)(2, 2))));
                    f32 scale = 0.5f / root;
                    rDest.x = 0.5f * root;
                    rDest.y = scale * ((*this)(0, 1) + (*this)(1, 0));
                    rDest.z = scale * ((*this)(2, 0) + (*this)(0, 2));
                    rDest.w = scale * ((*this)(2, 1) - (*this)(1, 2));
                } else if (maximum == (*this)(1, 1)) {
                    f32 root = std::sqrt(1.0f + ((*this)(1, 1) - ((*this)(2, 2) + (*this)(0, 0))));
                    f32 scale = 0.5f / root;
                    rDest.y = 0.5f * root;
                    rDest.z = scale * ((*this)(1, 2) + (*this)(2, 1));
                    rDest.x = scale * ((*this)(0, 1) + (*this)(1, 0));
                    rDest.w = scale * ((*this)(0, 2) - (*this)(2, 0));
                } else {
                    f32 root = std::sqrt(1.0f + ((*this)(2, 2) - ((*this)(0, 0) + (*this)(1, 1))));
                    f32 scale = 0.5f / root;
                    rDest.z = 0.5f * root;
                    rDest.x = scale * ((*this)(2, 0) + (*this)(0, 2));
                    rDest.y = scale * ((*this)(1, 2) + (*this)(2, 1));
                    rDest.w = scale * ((*this)(1, 0) - (*this)(0, 1));
                }
            }
        }

    f32 getRotate(TVec3f& axis) const { TQuat4<float> rotation; getQuat(rotation); return rotation.getRotate(axis); }
    template<class Q> void setQuat(const Q& q){
        const f32 yy=2.0f*q.y*q.y,zz=2.0f*q.z*q.z,xx=2.0f*q.x*q.x;
        const f32 xy=2.0f*q.x*q.y,xz=2.0f*q.x*q.z,yz=2.0f*q.y*q.z;
        const f32 wz=2.0f*q.w*q.z,wx=2.0f*q.w*q.x,wy=2.0f*q.w*q.y;
        this->mMtx[0][0]=1.0f-yy-zz;this->mMtx[0][1]=xy-wz;this->mMtx[0][2]=xz+wy;
        this->mMtx[1][0]=xy+wz;this->mMtx[1][1]=1.0f-xx-zz;this->mMtx[1][2]=yz-wx;
        this->mMtx[2][0]=xz-wy;this->mMtx[2][1]=yz+wx;this->mMtx[2][2]=1.0f-xx-yy;
    }
    void getEulerXYZ(TVec3f& dst)const{const float zx=this->mMtx[2][0],epsilon=32*std::numeric_limits<float>::epsilon(),halfPi=1.57079632679f;if(zx-1>=-epsilon){dst.set(::atan2f(-this->mMtx[0][1],this->mMtx[1][1]),-halfPi,0.0f);return;}if(zx+1<=epsilon){dst.set(::atan2f(this->mMtx[0][1],this->mMtx[1][1]),halfPi,0.0f);return;}dst.set(::atan2f(this->mMtx[2][1],this->mMtx[2][2]),std::asin(-zx),::atan2f(this->mMtx[1][0],this->mMtx[0][0]));} void getEuler(TVec3f& dst)const{getEulerXYZ(dst);} void getEulerDegree(TVec3f& dst)const{getEulerXYZ(dst);dst.scale(57.2957795131f);} void identity33(){for(int r=0;r<3;++r)for(int c=0;c<3;++c)this->mMtx[r][c]=(r==c)?1.0f:0.0f;}
    void getScale(TVec3f& dst)const{dst.set(std::sqrt(this->dotX()),std::sqrt(this->dotY()),std::sqrt(this->dotZ()));}
    void getXDir(TVec3f& dst)const{dst.set(this->mMtx[0][0],this->mMtx[1][0],this->mMtx[2][0]);}
    void getYDir(TVec3f& dst)const{dst.set(this->mMtx[0][1],this->mMtx[1][1],this->mMtx[2][1]);}
    void getYDir2(TVec3f& dst)const{getYDir(dst);}
    void getZDir2(TVec3f& dst)const{getZDir(dst);}
    void getZDir(TVec3f& dst)const{dst.set(this->mMtx[0][2],this->mMtx[1][2],this->mMtx[2][2]);}
    void setXYZDir(const TVec3f& x,const TVec3f& y,const TVec3f& z){this->mMtx[0][0]=x.x;this->mMtx[1][0]=x.y;this->mMtx[2][0]=x.z;this->mMtx[0][1]=y.x;this->mMtx[1][1]=y.y;this->mMtx[2][1]=y.z;this->mMtx[0][2]=z.x;this->mMtx[1][2]=z.y;this->mMtx[2][2]=z.z;}
    void setXYZDir2(const TVec3f& x,const TVec3f& y,const TVec3f& z){this->mMtx[0][0]=x.x;this->mMtx[0][1]=x.y;this->mMtx[0][2]=x.z;this->mMtx[1][0]=y.x;this->mMtx[1][1]=y.y;this->mMtx[1][2]=y.z;this->mMtx[2][0]=z.x;this->mMtx[2][1]=z.y;this->mMtx[2][2]=z.z;}
    void getXYZDir(TVec3f& x,TVec3f& y,TVec3f& z)const{getXDir(x);getYDir(y);getZDir(z);}
    void setXDir(f32 x,f32 y,f32 z){this->mMtx[0][0]=x;this->mMtx[1][0]=y;this->mMtx[2][0]=z;}
    void setYDir(f32 x,f32 y,f32 z){this->mMtx[0][1]=x;this->mMtx[1][1]=y;this->mMtx[2][1]=z;}
    void setZDir(f32 x,f32 y,f32 z){this->mMtx[0][2]=x;this->mMtx[1][2]=y;this->mMtx[2][2]=z;}
    void setXDir(const TVec3f& v){setXDir(v.x,v.y,v.z);}
    void setYDir(const TVec3f& v){setYDir(v.x,v.y,v.z);}
    void setZDir(const TVec3f& v){setZDir(v.x,v.y,v.z);}
    void mult33(const TVec3f& src,TVec3f& dst)const{const auto x=src.x,y=src.y,z=src.z;dst.set(x*this->get(0,0)+y*this->get(0,1)+z*this->get(0,2),x*this->get(1,0)+y*this->get(1,1)+z*this->get(1,2),x*this->get(2,0)+y*this->get(2,1)+z*this->get(2,2));}
    void mult33(TVec3f& v)const{mult33(v,v);}
};
template<class T> struct TPosition3:public TRotation3<T> {
    TPosition3()=default;
    void setEuler(const TVec3f& rotation){this->setRotate(rotation);}
    void makeRotateDegree(const TVec3f& axis,f32 degrees){makeRotate(axis,degrees*(3.14159265358979323846f/180.0f));}
    void setRT(const TVec3f& rotation,const TVec3f& translation){this->setRotate(rotation.x,rotation.y,rotation.z);setTrans(translation);}
    void setRT(f32 rx,f32 ry,f32 rz,const TVec3f& translation){this->setRotate(rx,ry,rz);setTrans(translation);}
    void makeRotate(const TVec3f& axis,f32 angle){this->zeroTrans();TVec3f v;v.normalize(axis);const f32 x=v.x,y=v.y,z=v.z,s=std::sin(angle),c=std::cos(angle),n=1-c;
        this->mMtx[0][0]=c+n*x*x;this->mMtx[0][1]=n*x*y-s*z;this->mMtx[0][2]=n*x*z+s*y;
        this->mMtx[1][0]=n*x*y+s*z;this->mMtx[1][1]=c+n*y*y;this->mMtx[1][2]=n*y*z-s*x;
        this->mMtx[2][0]=n*x*z-s*y;this->mMtx[2][1]=n*y*z+s*x;this->mMtx[2][2]=c+n*z*z;
    }
    void makeRotate(const TVec3f& from,const TVec3f& to,f32 maxAngle){TQuat4f q;q.setRotate(from,to,from.turnRate(to,maxAngle));makeQuat(q);}
    void normalizeBasis(){for(int column=0;column<3;++column){const f32 squared=this->mMtx[0][column]*this->mMtx[0][column]+this->mMtx[1][column]*this->mMtx[1][column]+this->mMtx[2][column]*this->mMtx[2][column];if(squared>32.0f*std::numeric_limits<f32>::epsilon()){const f32 inverse=1.0f/std::sqrt(squared);for(int row=0;row<3;++row)this->mMtx[row][column]*=inverse;}}}
    TPosition3(MtxPtr src){this->set(src);}
    TPosition3(const TPosition3* src){this->set(*src);}
    void getTrans(TVec3f& dst)const{dst.set(this->mMtx[0][3],this->mMtx[1][3],this->mMtx[2][3]);}
    void getTransInline2(TVec3f& dst)const{getTrans(dst);}
    void setTrans(f32 x,f32 y,f32 z){this->mMtx[0][3]=x;this->mMtx[1][3]=y;this->mMtx[2][3]=z;}
    void setTrans(const TVec3f& v){setTrans(v.x,v.y,v.z);}
    template<class Q> void setQT(const Q& quaternion,const TVec3f& translation){this->setQuat(quaternion);setTrans(translation);}
    void setTR(const TVec3f& x,const TVec3f& y,const TVec3f& z,const TVec3f& translation){this->setXYZDir(x,y,z);setTrans(translation);}
    void makeTrans(const TVec3f& v){this->identity();setTrans(v);}
    void makeTrans(f32 x,f32 y,f32 z){this->identity();setTrans(x,y,z);}
    void makeQuat(const TQuat4f& q){zeroTrans();this->setQuat(q);}
    void zeroTrans(){setTrans(0,0,0);}
    void zeroTransInline2(){zeroTrans();}
    void setPositionFromLookAt(const TPosition3& source){
        const TPosition3 src(source); // Preserve source when destination aliases it.
        this->mMtx[0][0]=-src.get(0,0);this->mMtx[1][1]=src.get(1,1);this->mMtx[2][2]=-src.get(2,2);
        this->mMtx[1][0]=-src.get(0,1);this->mMtx[0][1]=src.get(1,0);
        this->mMtx[2][0]=-src.get(0,2);this->mMtx[0][2]=-src.get(2,0);
        this->mMtx[1][2]=-src.get(2,1);this->mMtx[2][1]=src.get(1,2);
        TVec3f pos;src.getTrans(pos);
        for(int row=0;row<3;++row)this->mMtx[row][3]=pos.x*this->get(row,0)-pos.y*this->get(row,1)+pos.z*this->get(row,2);
    }
    void setPositionFromLookAt(const TVec3f& eye,const TVec3f& upDirection,const TVec3f& target){
        TVec3f front,side,up;front.sub(target,eye);
        auto normalize=[](TVec3f& v){v.scale(1.0f/std::sqrt(v.squared()));};
        normalize(front);front.negate();side.cross(upDirection,front);up.cross(front,side);normalize(side);normalize(up);
        this->mMtx[0][0]=side.x;this->mMtx[0][1]=side.y;this->mMtx[0][2]=side.z;
        this->mMtx[1][0]=up.x;this->mMtx[1][1]=up.y;this->mMtx[1][2]=up.z;
        this->mMtx[2][0]=front.x;this->mMtx[2][1]=front.y;this->mMtx[2][2]=front.z;
        this->mMtx[0][3]=eye.x*-side.x-eye.y*side.y-eye.z*side.z;
        this->mMtx[1][3]=eye.x*-up.x-eye.y*up.y-eye.z*up.z;
        this->mMtx[2][3]=eye.x*-front.x-eye.y*front.y-eye.z*front.z;
    }
};
}
using TSMtxf=JGeometry::SMatrix34C<f32>;
using TMtx34f=JGeometry::TMatrix34<TSMtxf>;
using TRot3f=JGeometry::TRotation3<TMtx34f>;
using TPos3f=JGeometry::TPosition3<TMtx34f>;
static_assert(sizeof(TMtx34f)==48 && sizeof(TPos3f)==48);

namespace JGeometry {
template<class T> struct SMatrix44C {T mMtx[4][4];using ArrType=T[4];void set(const ArrType* src){for(int r=0;r<4;++r)for(int c=0;c<4;++c)mMtx[r][c]=src[r][c];}void set(const SMatrix44C& src){set(src.mMtx);}T get(int r,int c)const{return mMtx[r][c];}T operator()(int r,int c)const{return get(r,c);}operator ArrType*(){return mMtx;}operator const ArrType*()const{return mMtx;}};
template<class T> struct TMatrix44:public T {
    Mtx44Ptr toMtx44(){return this->mMtx;}
    void identity(){for(int r=0;r<4;++r)for(int c=0;c<4;++c)this->mMtx[r][c]=r==c?1.0f:0.0f;}
    void concat(const T& a,const T& b){
        // Preserve original grouping; stage every result before writing aliased operands.
        T result;
        for(int r=0;r<4;++r)for(int c=0;c<4;++c)
            result.mMtx[r][c]=a.mMtx[r][3]*b.mMtx[3][c]+(a.mMtx[r][2]*b.mMtx[2][c]+(a.mMtx[r][0]*b.mMtx[0][c]+a.mMtx[r][1]*b.mMtx[1][c]));
        this->set(result);
    }
};
template<class T> struct TProjection3:public T {
    void makeTrans(float x,float y){this->identity();this->mMtx[0][3]=x;this->mMtx[1][3]=y;}
    void makeTrans(const TVec2f& offset){makeTrans(offset.x,offset.y);}
    TProjection3()=default;explicit TProjection3(Mtx44Ptr src){this->set(src);}
    float getPower(float fov){return std::tan(3.1415927f*fov/360.0f);}float getFocalLength(float fov){return 1.0f/getPower(fov);}
    void makePerspective(float fov,float aspect,float nearPlane,float farPlane){const float focal=getFocalLength(fov),scale=1.0f/(farPlane-nearPlane);for(auto& row:this->mMtx)for(auto& v:row)v=0;this->mMtx[0][0]=focal/aspect;this->mMtx[1][1]=focal;this->mMtx[2][2]=-nearPlane*scale;this->mMtx[2][3]=-(farPlane*nearPlane)*scale;this->mMtx[3][2]=-1;}
};
}
using TSMtx44f=JGeometry::SMatrix44C<float>;using TMtx44f=JGeometry::TMatrix44<TSMtx44f>;using TProj3f=JGeometry::TProjection3<TMtx44f>;
static_assert(sizeof(TProj3f)==64);

















