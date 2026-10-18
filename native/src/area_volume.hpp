#pragma once
#include "stage_placement.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include <functional>
class AreaObj;
void bindNativeAreaVolume(const AreaObj*,std::function<bool(const TVec3f&)>);
void unbindNativeAreaVolume(const AreaObj*);
class NativeAreaVolumeBinding{const AreaObj* owner;public:NativeAreaVolumeBinding(const AreaObj* area,std::function<bool(const TVec3f&)> contains):owner(area){bindNativeAreaVolume(owner,std::move(contains));}~NativeAreaVolumeBinding(){unbindNativeAreaVolume(owner);}NativeAreaVolumeBinding(const NativeAreaVolumeBinding&)=delete;NativeAreaVolumeBinding& operator=(const NativeAreaVolumeBinding&)=delete;};
inline bool nativeBaseCubeContains(const AffineMatrix& world,const TVec3f& position){
    std::array<double,3> a{world[0],world[4],world[8]},b{world[1],world[5],world[9]},c{world[2],world[6],world[10]},d{double(position.x)-world[3],double(position.y)-world[7],double(position.z)-world[11]};
    auto cross=[](const auto& x,const auto& y){return std::array<double,3>{x[1]*y[2]-x[2]*y[1],x[2]*y[0]-x[0]*y[2],x[0]*y[1]-x[1]*y[0]};};auto dot=[](const auto& x,const auto& y){return x[0]*y[0]+x[1]*y[1]+x[2]*y[2];};auto bc=cross(b,c);double det=dot(a,bc);assetCheck(std::isfinite(det)&&std::abs(det)>1e-12,"Singular native area transform");double x=dot(d,bc)/det,y=dot(d,cross(c,a))/det,z=dot(d,cross(a,b))/det;
    return x>=-500&&x<500&&y>=0&&y<1000&&z>=-500&&z<500;
}
inline bool nativeDeathVolumeContains(int form,const AffineMatrix& world,const TVec3f& p){
    if(form==1)return nativeBaseCubeContains(world,p);
    double dx=double(p.x)-world[3],dy=double(p.y)-world[7],dz=double(p.z)-world[11];double radius2=250000*(double(world[0])*world[0]+double(world[4])*world[4]+double(world[8])*world[8]);
    if(form==2)return dx*dx+dy*dy+dz*dz<radius2;
    assetCheck(form==3,"Unsupported death-area form");double upLength=std::sqrt(double(world[1])*world[1]+double(world[5])*world[5]+double(world[9])*world[9]);assetCheck(upLength>0,"Zero death cylinder height");double height=(dx*world[1]+dy*world[5]+dz*world[9])/upLength;
    double radial=dx*dx+dy*dy+dz*dz-height*height;return height>=0&&height<=1000*upLength&&radial<radius2;
}
