#include <native_no_weak.h>
#include <revolution/mtx.h>
#include <cmath>
// Scalar host implementations. Console paired-single estimate/rounding parity is unverified.
extern "C" void PSMTXQuat(Mtx dst,const Quaternion* quaternion){
    // SDK assembly uses 2 / squared norm, so non-unit inputs also rotate.
    const float x=quaternion->x,y=quaternion->y,z=quaternion->z,w=quaternion->w;
    const float scale=2.0f/((x*x+z*z)+(y*y+w*w));
    dst[0][0]=1.0f-scale*(y*y+z*z);
    dst[0][1]=scale*(x*y-z*w);
    dst[0][2]=scale*(x*z+y*w);
    dst[1][0]=scale*(x*y+z*w);
    dst[1][1]=1.0f-scale*(x*x+z*z);
    dst[1][2]=scale*(y*z-x*w);
    dst[2][0]=scale*(x*z-y*w);
    dst[2][1]=scale*(y*z+x*w);
    dst[2][2]=1.0f-scale*(x*x+y*y);
    dst[0][3]=dst[1][3]=dst[2][3]=0;
}
extern "C" void PSVECNormalize(const Vec* src,Vec* dst){
    const float x=src->x,y=src->y,z=src->z;
    const float inverse=1.0f/std::sqrt(x*x+y*y+z*z);
    const Vec result{x*inverse,y*inverse,z*inverse};
    *dst=result;
}
extern "C" f32 PSVECMag(const Vec* src){
    const float x=src->x,y=src->y,z=src->z;
    return std::sqrt((x*x+y*y)+z*z);
}
extern "C" void PSQUATMultiply(const Quaternion* a,const Quaternion* b,Quaternion* dst){
    const float ax=a->x,ay=a->y,az=a->z,aw=a->w;
    const float bx=b->x,by=b->y,bz=b->z,bw=b->w;
    const Quaternion product{
        aw*bx+ax*bw+ay*bz-az*by,
        aw*by-ax*bz+ay*bw+az*bx,
        aw*bz+ax*by-ay*bx+az*bw,
        aw*bw-ax*bx-ay*by-az*bz
    };
    *dst=product;
}
extern "C" void PSMTXTrans(Mtx dst,f32 x,f32 y,f32 z){
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)dst[r][c]=(r==c)?1.0f:0.0f;
    dst[0][3]=x;dst[1][3]=y;dst[2][3]=z;
}
extern "C" void PSMTXConcat(const Mtx a,const Mtx b,Mtx dst){
    Mtx result;
    for(int r=0;r<3;++r){
        for(int c=0;c<3;++c)result[r][c]=a[r][0]*b[0][c]+a[r][1]*b[1][c]+a[r][2]*b[2][c];
        result[r][3]=a[r][0]*b[0][3]+a[r][1]*b[1][3]+a[r][2]*b[2][3]+a[r][3];
    }
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)dst[r][c]=result[r][c];
}
extern "C" void PSMTXMultVecSR(const Mtx m,const Vec* src,Vec* dst){
    const float x=src->x,y=src->y,z=src->z;
    const Vec result{m[0][0]*x+m[0][1]*y+m[0][2]*z,m[1][0]*x+m[1][1]*y+m[1][2]*z,m[2][0]*x+m[2][1]*y+m[2][2]*z};
    *dst=result;
}
extern "C" void PSMTXRotAxisRad(Mtx m,const Vec* axis,f32 rad){
    const float inverse=1.0f/std::sqrt(axis->x*axis->x+axis->y*axis->y+axis->z*axis->z);
    const float x=axis->x*inverse,y=axis->y*inverse,z=axis->z*inverse;
    const float s=std::sin(rad),c=std::cos(rad),t=1-c;
    m[0][0]=t*x*x+c;m[0][1]=t*x*y-s*z;m[0][2]=t*x*z+s*y;m[0][3]=0;
    m[1][0]=t*x*y+s*z;m[1][1]=t*y*y+c;m[1][2]=t*y*z-s*x;m[1][3]=0;
    m[2][0]=t*x*z-s*y;m[2][1]=t*y*z+s*x;m[2][2]=t*z*z+c;m[2][3]=0;
}
extern "C" void PSMTXMultVec(const Mtx m,const Vec* src,Vec* dst){
    const float x=src->x,y=src->y,z=src->z;
    const Vec result{m[0][0]*x+m[0][1]*y+m[0][2]*z+m[0][3],m[1][0]*x+m[1][1]*y+m[1][2]*z+m[1][3],m[2][0]*x+m[2][1]*y+m[2][2]*z+m[2][3]};
    *dst=result;
}
extern "C" void PSVECCrossProduct(const Vec* a,const Vec* b,Vec* dst){
    const Vec result{a->y*b->z-a->z*b->y,a->z*b->x-a->x*b->z,a->x*b->y-a->y*b->x};
    *dst=result;
}

extern "C" u32 PSMTXInverse(const Mtx src,Mtx dst){
 const float a=src[0][0],b=src[0][1],c=src[0][2],d=src[1][0],e=src[1][1],f=src[1][2],g=src[2][0],h=src[2][1],i=src[2][2];
 const float A=e*i-f*h,B=c*h-b*i,C=b*f-c*e,D=f*g-d*i,E=a*i-c*g,F=c*d-a*f,G=d*h-e*g,H=b*g-a*h,I=a*e-b*d;
 const float determinant=a*A+b*D+c*G;if(determinant==0||!std::isfinite(determinant))return 0;
 const float inverse=1.0f/determinant;Mtx result={{A*inverse,B*inverse,C*inverse,0},{D*inverse,E*inverse,F*inverse,0},{G*inverse,H*inverse,I*inverse,0}};
 for(int r=0;r<3;++r)result[r][3]=-(result[r][0]*src[0][3]+result[r][1]*src[1][3]+result[r][2]*src[2][3]);
 for(int r=0;r<3;++r)for(int col=0;col<4;++col)dst[r][col]=result[r][col];return 1;
}
#include <cstring>
extern "C" void PSMTXIdentity(Mtx dst){for(int r=0;r<3;++r)for(int c=0;c<4;++c)dst[r][c]=r==c?1.0f:0.0f;}
extern "C" void PSMTXCopy(const Mtx src,Mtx dst){std::memmove(dst,src,sizeof(Mtx));}
extern "C" void PSMTXScale(Mtx dst,f32 x,f32 y,f32 z){PSMTXIdentity(dst);dst[0][0]=x;dst[1][1]=y;dst[2][2]=z;}
extern "C" void PSMTXRotRad(Mtx dst,char axis,f32 radians){
    const char normalizedAxis=axis|0x20;
    if(normalizedAxis!='x'&&normalizedAxis!='y'&&normalizedAxis!='z')return;
    const float sine=std::sin(radians),cosine=std::cos(radians);
    PSMTXIdentity(dst);
    switch(normalizedAxis){
    case 'x': dst[1][1]=cosine;dst[1][2]=-sine;dst[2][1]=sine;dst[2][2]=cosine;break;
    case 'y': dst[0][0]=cosine;dst[0][2]=sine;dst[2][0]=-sine;dst[2][2]=cosine;break;
    case 'z': dst[0][0]=cosine;dst[0][1]=-sine;dst[1][0]=sine;dst[1][1]=cosine;break;
    default: break;
    }
}
extern "C" void PSMTXScaleApply(const Mtx src,Mtx dst,f32 x,f32 y,f32 z){
    const f32 scale[3]={x,y,z};Mtx result;
    for(int r=0;r<3;++r)for(int c=0;c<4;++c)result[r][c]=src[r][c]*scale[r];
    std::memcpy(dst,result,sizeof(Mtx));
}
extern "C" void PSMTXTransApply(const Mtx src,Mtx dst,f32 x,f32 y,f32 z){Mtx result;std::memcpy(result,src,sizeof(Mtx));result[0][3]+=x;result[1][3]+=y;result[2][3]+=z;std::memcpy(dst,result,sizeof(Mtx));}
extern "C" void PSMTX44Identity(Mtx44 dst){for(int r=0;r<4;++r)for(int c=0;c<4;++c)dst[r][c]=r==c?1.0f:0.0f;}
extern "C" void PSMTX44Copy(const Mtx44 src,Mtx44 dst){std::memmove(dst,src,sizeof(Mtx44));}
extern "C" void PSVECScale(const Vec* src,Vec* dst,f32 scale){const float x=src->x*scale,y=src->y*scale,z=src->z*scale;dst->x=x;dst->y=y;dst->z=z;}
extern "C" f32 C_VECMag(const Vec* v){return std::sqrt((v->x*v->x)+(v->y*v->y)+(v->z*v->z));}
namespace JMathInlineVEC {
f32 PSVECSquareDistance(const Vec* a,const Vec* b){
    const f32 dx=a->x-b->x,dy=a->y-b->y,dz=a->z-b->z;
    return (dx*dx+dy*dy)+dz*dz;
}
}

extern "C" void C_MTXLightPerspective(Mtx m, f32 fovY, f32 aspect, float scaleS, float scaleT, float transS, float transT) {
    f32 angle;
    f32 cot;
    angle = fovY * 0.5f;
    angle = MTXDegToRad(angle);
    cot = tan(angle);
    cot = 1.0f / cot;
    m[0][0] = (cot / aspect) * scaleS;
    m[0][1] = 0.0f;
    m[0][2] = -transS;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = cot * scaleT;
    m[1][2] = -transT;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = -1.0f;
    m[2][3] = 0.0f;
}
extern "C" void C_MTXLightOrtho(Mtx m, f32 t, f32 b, f32 l, f32 r, float scaleS, float scaleT, float transS, float transT) {
    f32 tmp;
    tmp = 1.0f / (r - l);
    m[0][0] = (2.0f * tmp * scaleS);
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = ((-(r + l) * tmp) * scaleS) + transS;
    tmp = 1.0f / (t - b);
    m[1][0] = 0.0f;
    m[1][1] = (2.0f * tmp) * scaleT;
    m[1][2] = 0.0f;
    m[1][3] = ((-(t + b) * tmp) * scaleT) + transT;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 0.0f;
    m[2][3] = 1.0f;
}
