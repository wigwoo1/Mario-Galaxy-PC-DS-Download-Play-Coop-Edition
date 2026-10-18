#pragma once
#include <revolution/mtx.h>
#include <type_traits>
#include <cmath>
#include <limits>
namespace JGeometry {
template<class T> struct TVec3 {
    T x,y,z;
    TVec3():x(0),y(0),z(0){}
    explicit TVec3(T value):x(value),y(value),z(value){}
    TVec3(T a,T b,T c):x(a),y(b),z(c){}
    TVec3(float xz,float vertical):x(T(xz)),y(T(vertical)),z(T(xz)){}
    TVec3(const Vec& v):x(T(v.x)),y(T(v.y)),z(T(v.z)){static_assert(std::is_same<T,float>::value);}
    void set(const Vec& v){static_assert(std::is_same<T,float>::value);set(v.x,v.y,v.z);}
    template<class A,class B,class C> void set(A a,B b,C c){x=T(a);y=T(b);z=T(c);}
    void set(const TVec3& v){*this=v;}
    void set(T value){x=y=z=value;}
    template<class U> void setAll(U value){x=y=z=T(value);}
    void zero(){x=y=z=0;}
    void setPSZeroVec(){zero();}
    void sub(const TVec3& a,const TVec3& b){set(a.x-b.x,a.y-b.y,a.z-b.z);}
    void sub(const TVec3& b){sub(*this,b);}
    void add(const TVec3& v){x+=v.x;y+=v.y;z+=v.z;}
    void add(const TVec3& a,const TVec3& b){set(a.x+b.x,a.y+b.y,a.z+b.z);}
    void scale(float value){x=T(x*value);y=T(y*value);z=T(z*value);}
    void scale(float value,const TVec3& v){set(T(v.x*value),T(v.y*value),T(v.z*value));}
    void mult(float value){scale(value);} void scaleAdd(float value,const TVec3& source){set(T(x+source.x*value),T(y+source.y*value),T(z+source.z*value));} void scaleAdd(float value,const TVec3& a,const TVec3& b){set(T(a.x*value+b.x),T(a.y*value+b.y),T(a.z*value+b.z));}
    TVec3 multInLine(float value)const{TVec3 result(*this);result.scale(value);return result;}
    TVec3 multInLine2(float value)const{return multInLine(value);}
    TVec3 scaleInline(float value)const{return multInLine(value);}
    void mul(const TVec3& a,const TVec3& b){set(a.x*b.x,a.y*b.y,a.z*b.z);}
    void mul(const TVec3& v){mul(*this,v);}
    void negate(const TVec3& v){set(-v.x,-v.y,-v.z);}
    void negate(){negate(*this);}
    float dot(const TVec3& v)const{return float(x*v.x+y*v.y+z*v.z);}
    void orthogonalize(const TVec3& direction){scaleAdd(-direction.dot(*this),direction,*this);}
    void killElement(const TVec3& source,const TVec3& direction){scaleAdd(-direction.dot(source),direction,source);}
    void killElement2(const TVec3& source,const TVec3& direction){scaleAdd(-direction.dot(source),direction,source);}
    bool orientation(const TVec3& a,const TVec3& b)const{return a.cross(b).dot(*this)<0.0f;}
    TVec3 getOrthogonal(const TVec3& source)const{TVec3 result;result.killElement(source,*this);return result;}
    TVec3 killElement(const TVec3& direction)const{TVec3 result;result.killElement(*this,direction);return result;}
    // Convenience value copy used by original actor code to preserve the pre-update direction.
    TVec3 copy()const{return *this;}
    void orthogonalize2(const TVec3& direction){scaleAdd(-direction.dot(*this),direction,*this);}
    float squared()const{return dot(*this);}
    float squared(const TVec3& v)const{TVec3 delta;delta.sub(*this,v);return delta.squared();}
    float length()const{return std::sqrt(squared());}
    float normalize(){const float magnitude=length();scale(1.0f/magnitude);return magnitude;}
    float normalize(const TVec3& source){set(source);return normalize();}
    void setTrans(MtxPtr matrix){set(matrix[0][3],matrix[1][3],matrix[2][3]);}
    float distance(const TVec3& v)const{return std::sqrt(squared(v));}
    float angle(const TVec3& v)const{return std::fabs(std::atan2(cross(v).length(),dot(v)));}
    float turnRate(const TVec3& v,float maxAngle)const{const float current=angle(v);return current>maxAngle?maxAngle/current:1.0f;}
    bool isZero()const{return squared()<=32*std::numeric_limits<float>::epsilon();}
    void cross(const TVec3& a,const TVec3& b){const auto cx=a.y*b.z-a.z*b.y,cy=a.z*b.x-a.x*b.z,cz=a.x*b.y-a.y*b.x;set(cx,cy,cz);}
    TVec3 cross(const TVec3& b)const{TVec3 out;out.cross(*this,b);return out;}
    // Host square root; console estimate/rounding parity remains unverified.
    float setLength(float target){const auto old=squared();if(old<=32*std::numeric_limits<float>::epsilon())return 0;const auto inverse=1.0f/std::sqrt(old);scale(inverse*target);return inverse*old;}
    float setLength(const TVec3& source,float target){const auto old=source.squared();if(old<=32*std::numeric_limits<float>::epsilon()){zero();return 0;}const auto inverse=1.0f/std::sqrt(old);scale(inverse*target,source);return inverse*old;}
    void lerp(const TVec3& a,const TVec3& b,float t){set(T(a.x+(b.x-a.x)*t),T(a.y+(b.y-a.y)*t),T(a.z+(b.z-a.z)*t));}
    template<class U> void cubic(const TVec3& p0,const TVec3& v0,const TVec3& v1,const TVec3& p1,U t){const U t2=t*t,t3=t2*t,h00=U(2)*t3-U(3)*t2+U(1),h01=-U(2)*t3+U(3)*t2,h10=t3-U(2)*t2+t,h11=t3-t2;set(T(h00*p0.x+h01*p1.x+h10*v0.x+h11*v1.x),T(h00*p0.y+h01*p1.y+h10*v0.y+h11*v1.y),T(h00*p0.z+h01*p1.z+h10*v0.z+h11*v1.z));}
    TVec3 operator+(const TVec3& v)const{TVec3 out;out.add(*this,v);return out;}
    TVec3 operator-(const TVec3& v)const{TVec3 out;out.sub(*this,v);return out;}
    TVec3 operator-()const{TVec3 out;out.negate(*this);return out;}
    TVec3 operator*(float value)const{return multInLine(value);} TVec3 operator/(float value)const{return multInLine(1.0f/value);} void operator/=(float value){scale(1.0f/value);} operator Vec()const{static_assert(std::is_same<T,float>::value);return Vec{x,y,z};} operator Vec*(){static_assert(std::is_same<T,float>::value);return reinterpret_cast<Vec*>(&x);} operator const Vec*()const{static_assert(std::is_same<T,float>::value);return reinterpret_cast<const Vec*>(&x);}
    TVec3 operator*(const TVec3& v)const{TVec3 out;out.mul(*this,v);return out;}
    void operator+=(const TVec3& v){add(v);}
    void operator-=(const TVec3& v){sub(v);}
    void operator*=(float value){scale(value);}
    void operator*=(const TVec3& v){mul(v);}
};
}
using TVec3f=JGeometry::TVec3<float>;
inline void JMAVECScaleAdd(const TVec3f* a,const TVec3f* b,TVec3f* dst,f32 scale){const f32 x=a->x+b->x*scale,y=a->y+b->y*scale,z=a->z+b->z*scale;dst->set(x,y,z);}

namespace JGeometry {
template<class T> struct TVec2 {
    T x,y;
    TVec2()=default;
    template<class U> TVec2(U a,U b):x(T(a)),y(T(b)){}
    template<class A,class B> void set(A a,B b){x=T(a);y=T(b);}
    template<class U> void set(const TVec2<U>& v){set(v.x,v.y);}
    void scale(T value){x=T(x*value);y=T(y*value);}
    void set(T value){x=y=value;}
    void add(const TVec2& v){x+=v.x;y+=v.y;}
    void operator+=(const TVec2& v){add(v);}
    TVec2 operator*(T value)const{return TVec2(x*value,y*value);}
    TVec2 operator+(const TVec2& v)const{TVec2 result(*this);result.add(v);return result;}
    void setMin(const TVec2<float>& v){if(x>=v.x)x=T(v.x);if(y>=v.y)y=T(v.y);}
    void setMax(const TVec2<float>& v){if(x<=v.x)x=T(v.x);if(y<=v.y)y=T(v.y);}
    bool isAbove(const TVec2& v)const{return x>=v.x&&y>=v.y;}
    bool isZero()const{return squared()<=T(32.0f*std::numeric_limits<float>::epsilon());}
    void zero(){x=y=T(0);}
    void sub(const TVec2& v){x-=v.x;y-=v.y;}
    void sub(const TVec2& a,const TVec2& b){const T nx=a.x-b.x,ny=a.y-b.y;x=nx;y=ny;}
    TVec2 operator-(const TVec2& v)const{TVec2 result(*this);result.sub(v);return result;}
    TVec2 subOperatorInline(const TVec2& v)const{TVec2 result(*this);result.sub(v);return result;}
    T dot(const TVec2& v)const{return x*v.x+y*v.y;}
    T squareDist(const TVec2& v)const{const T dx=x-v.x,dy=y-v.y;return dx*dx+dy*dy;}
    T distance(const TVec2& other)const{const T dx=x-other.x,dy=y-other.y;return T(std::sqrt(dx*dx+dy*dy));}
    T squared()const{return dot(*this);}
    T squared(const TVec2& other)const;
    T length()const{return T(std::sqrt(squared()));}
    float setLength(float target){const float old=float(squared());if(old<=32.0f*std::numeric_limits<float>::epsilon())return 0.0f;const float inverse=1.0f/std::sqrt(old);x=T(x*inverse*target);y=T(y*inverse*target);return inverse*old;}
    float normalize(){return setLength(1.0f);}
};
}
using TVec2f=JGeometry::TVec2<float>;

#include <revolution/mtx.h>
#include <cmath>
#include <limits>
namespace JGeometry {
template<class T> struct TVec4 {
    T x,y,z,w;
    TVec4()=default;
    TVec3<T>* toTVec3(){return reinterpret_cast<TVec3<T>*>(this);}
    const TVec3<T>* toTVec3()const{return reinterpret_cast<const TVec3<T>*>(this);}
    template<class U> TVec4(U a,U b,U c,U d):x(T(a)),y(T(b)),z(T(c)),w(T(d)){}
    template<class U> void set(U a,U b,U c,U d){x=T(a);y=T(b);z=T(c);w=T(d);}
    template<class U> void set(const TVec4<U>& v){set(v.x,v.y,v.z,v.w);}
    void scale(T value){x*=value;y*=value;z*=value;w*=value;}
    operator Quaternion*(){static_assert(sizeof(T)==sizeof(f32));return reinterpret_cast<Quaternion*>(&x);}
    operator const Quaternion*()const{static_assert(sizeof(T)==sizeof(f32));return reinterpret_cast<const Quaternion*>(&x);}
};
template<class T> struct TQuat4:public TVec4<T> {
    TQuat4()=default;
    TQuat4(const TVec4<T>& q){this->set(q.x,q.y,q.z,q.w);}
    TQuat4(T xyz,T scalar){this->set(xyz,xyz,xyz,scalar);}
    template<class U> TQuat4(U x,U y,U z,U w){this->set(x,y,z,w);}
    TQuat4(const Quaternion& q){this->set(q.x,q.y,q.z,q.w);}
    void setRotate(const TVec3<T>& a,const TVec3<T>& b){const auto dir=a.cross(b);const float crossPart=dir.length();if(crossPart<=32*std::numeric_limits<float>::epsilon()){this->set(T(0),T(0),T(0),T(1));}else{const float halfAngle=0.5f*std::atan2f(crossPart,float(a.dot(b)));const float scale=std::sin(halfAngle)/crossPart;this->set(T(dir.x*scale),T(dir.y*scale),T(dir.z*scale),T(std::cos(halfAngle)));}}
    void setRotate(const TVec3<T>& a,const TVec3<T>& b,float ratio){const auto dir=a.cross(b);const float crossPart=dir.length();if(crossPart<=32*std::numeric_limits<float>::epsilon()){this->set(T(0),T(0),T(0),T(1));}else{const float halfAngle=ratio*(0.5f*std::atan2f(crossPart,float(a.dot(b))));const float scale=std::sin(halfAngle)/crossPart;this->set(T(dir.x*scale),T(dir.y*scale),T(dir.z*scale),T(std::cos(halfAngle)));}}
    float dot(const TQuat4& q)const{return this->x*q.x+this->y*q.y+this->z*q.z+this->w*q.w;}
    float squared()const{return dot(*this);}
    // Host sqrt replaces the console estimate/refinement; bit-exact parity is unverified.
    void normalize(const TQuat4& src){const auto length=src.squared();if(length<=32*std::numeric_limits<float>::epsilon()){this->set(T(0),T(0),T(0),T(1));return;}const auto inverse=1.0f/std::sqrt(length);this->set(T(src.x*inverse),T(src.y*inverse),T(src.z*inverse),T(src.w*inverse));}
    void slerp(const TQuat4& other,T ratio){TQuat4 a,b;a.normalize(*this);b.normalize(other);float product=a.dot(b);const bool reverse=product<0;if(reverse)product=-product;float weight;if(1-product<=32*std::numeric_limits<float>::epsilon())weight=1-ratio;else{const float angle=std::acos(product),s=std::sin(angle);weight=std::sin((1-ratio)*angle)/s;ratio=std::sin(ratio*angle)/s;}if(reverse)ratio=-ratio;this->set(weight*a.x+ratio*b.x,weight*a.y+ratio*b.y,weight*a.z+ratio*b.z,weight*a.w+ratio*b.w);}
    void slerp(const TQuat4& a,const TQuat4& b,T ratio){this->set(a.x,a.y,a.z,a.w);slerp(b,ratio);}
    void normalize(){normalize(*this);}
    void mult(const TQuat4& a,const TQuat4& b){
        const auto x=a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y;
        const auto y=a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x;
        const auto z=a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w;
        const auto w=a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z;
        this->set(x,y,z,w);
    }
    // Original one-argument form pre-multiplies by q.
    void mult(const TQuat4& q){mult(q,*this);}
    void transform(const TVec3<T>& v,TVec3<T>& dst)const{
        TQuat4 r;r.x=this->y*v.z-this->z*v.y+this->w*v.x;r.y=-this->x*v.z+this->z*v.x+this->w*v.y;r.z=this->x*v.y-this->y*v.x+this->w*v.z;r.w=-this->x*v.x-this->y*v.y-this->z*v.z;
        dst.set(r.x*this->w+r.y*-this->z-r.z*-this->y+r.w*-this->x,-r.x*-this->z+r.y*this->w+r.z*-this->x+r.w*-this->y,r.x*-this->y-r.y*-this->x+r.z*this->w+r.w*-this->z);
    }
    void transform(TVec3<T>& v)const{transform(v,v);}
    float getRotate(TVec3<T>& axis) const {
        const float length=this->x*this->x+this->y*this->y+this->z*this->z;
        if(length<=32*std::numeric_limits<float>::epsilon()){axis.zero();return 0.0f;}
        const float inv=1.0f/std::sqrt(length);
        axis.set(this->x*inv,this->y*inv,this->z*inv);
        return 2.0f*std::acos(this->w);
    }
    void setRotate(const TVec3<T>& axis,float angle){const float half=angle*0.5f;const float scale=std::sin(half);this->set(T(axis.x*scale),T(axis.y*scale),T(axis.z*scale),T(std::cos(half)));}
    void setRotate(float x,float y,float z,float angle){setRotate(TVec3<T>(T(x),T(y),T(z)),angle);}
    void rotate(TVec3<T>& v)const{transform(v,v);}
    void getXDir(TVec3<T>& dst)const{dst.set(T(1)-this->y*this->y*T(2)-this->z*this->z*T(2),this->x*this->y*T(2)+this->w*this->z*T(2),this->x*this->z*T(2)-this->w*this->y*T(2));}
    void getYDir(TVec3<T>& dst)const{dst.set(this->x*this->y*T(2)-this->w*this->z*T(2),T(1)-this->x*this->x*T(2)-this->z*this->z*T(2),this->y*this->z*T(2)+this->w*this->x*T(2));}
    void getZDir(TVec3<T>& dst)const{dst.set(this->x*this->z*T(2)+this->w*this->y*T(2),this->y*this->z*T(2)-this->w*this->x*T(2),T(1)-this->x*this->x*T(2)-this->y*this->y*T(2));}
    void setEuler(T x,T y,T z){const float cx=std::cos(x*0.5f),cy=std::cos(y*0.5f),cz=std::cos(z*0.5f),sx=std::sin(x*0.5f),sy=std::sin(y*0.5f),sz=std::sin(z*0.5f);const float cycz=cy*cz,sycz=sy*cz,cysz=cy*sz,sysz=sy*sz;this->set(T(cycz*sx-sysz*cx),T(sycz*cx+cysz*sx),T(cysz*cx-sycz*sx),T(cycz*cx+sysz*sx));}
    void setEulerZ(T z){const float half=z*0.5f;this->set(T(0),T(0),T(std::sin(half)),T(std::cos(half)));}
    void setEuler(const TVec3<T>& rpy){setEuler(rpy.x,rpy.y,rpy.z);}
    void setEulerDegree(T x,T y,T z){constexpr float degreesToRadians=0.017453292f;setEuler(T(x*degreesToRadians),T(y*degreesToRadians),T(z*degreesToRadians));}
    void makeMtx(MtxPtr dst)const{TVec3<T> x,y,z;getXDir(x);getYDir(y);getZDir(z);dst[0][0]=x.x;dst[1][0]=x.y;dst[2][0]=x.z;dst[0][1]=y.x;dst[1][1]=y.y;dst[2][1]=y.z;dst[0][2]=z.x;dst[1][2]=z.y;dst[2][2]=z.z;}
};
}
using TVec4f=JGeometry::TVec4<float>;
using TQuat4f=JGeometry::TQuat4<float>;
static_assert(sizeof(TQuat4f)==16);

// Preserve original pointer-style SDK calls, including in-place multiplication.
inline void PSQUATMultiply(const TQuat4f* a,const TQuat4f* b,TQuat4f* dst){
    TQuat4f product;product.mult(*a,*b);*dst=product;
}

using TVec2s=JGeometry::TVec2<s16>;
using TVec3s=JGeometry::TVec3<s16>;
using TVec3Sc=JGeometry::TVec3<s8>;


// Explicit SDK pointer adapters copy values rather than truncate or reinterpret pointers.
inline void PSMTXRotAxisRad(Mtx dst,const TVec3f* axis,f32 rad){const Vec nativeAxis{axis->x,axis->y,axis->z};PSMTXRotAxisRad(dst,&nativeAxis,rad);}
inline void PSMTXMultVecSR(const Mtx matrix,const TVec3f* src,TVec3f* dst){const Vec nativeSource{src->x,src->y,src->z};Vec nativeResult;PSMTXMultVecSR(matrix,&nativeSource,&nativeResult);dst->set(nativeResult.x,nativeResult.y,nativeResult.z);}

inline void PSMTXMultVec(const Mtx matrix,const TVec3f* src,TVec3f* dst){const Vec nativeSource{src->x,src->y,src->z};Vec nativeResult;PSMTXMultVec(matrix,&nativeSource,&nativeResult);dst->set(nativeResult.x,nativeResult.y,nativeResult.z);}
inline void PSMTXMultVec(const Mtx matrix,const Vec* src,TVec3f* dst){Vec result;PSMTXMultVec(matrix,src,&result);dst->set(result.x,result.y,result.z);}
inline void PSMTXMultVec(const Mtx matrix,const TVec3f* src,Vec* dst){const Vec nativeSource{src->x,src->y,src->z};PSMTXMultVec(matrix,&nativeSource,dst);}
inline void PSMTXMultVec(const Mtx matrix,const TVec3f* src,TVec3f& dst){PSMTXMultVec(matrix,src,&dst);}
inline void PSMTXMultVecSR(const Mtx matrix,const TVec3f& src,TVec3f* dst){PSMTXMultVecSR(matrix,&src,dst);}
inline void PSVECCrossProduct(const TVec3f* a,const TVec3f* b,TVec3f* dst){const Vec nativeA{a->x,a->y,a->z},nativeB{b->x,b->y,b->z};Vec result;PSVECCrossProduct(&nativeA,&nativeB,&result);dst->set(result.x,result.y,result.z);}




inline void PSMTXMultVec(const Mtx matrix,const TVec3f& src,TVec3f* dst){PSMTXMultVec(matrix,&src,dst);}







inline void PSMTXMultVecSR(const Mtx matrix,const TVec3f* src,Vec* dst){const Vec source{src->x,src->y,src->z};PSMTXMultVecSR(matrix,&source,dst);}
inline void PSMTXMultVecArraySR(const Mtx matrix,const TVec3f* src,TVec3f* dst,u32 count){for(u32 i=0;i<count;++i)PSMTXMultVecSR(matrix,src+i,dst+i);}









