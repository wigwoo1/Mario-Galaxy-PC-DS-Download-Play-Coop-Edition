#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <array>
#include <cmath>
#include <stdexcept>
namespace {
std::array<float,6> retained{};bool loaded=false;
float zScale=16777216.f,zOffset=0;
void emit(const std::array<float,6>& v,float scale,float offset){
 for(float f:v)if(!std::isfinite(f))throw std::invalid_argument("Nonfinite GX viewport");
 if(v[2]<=0||v[3]<=0)throw std::invalid_argument("Invalid GX viewport extent");
 std::array<float,6> xf={v[2]/2,-v[3]/2,(v[5]-v[4])*scale,v[0]+v[2]/2+342,v[1]+v[3]/2+342,v[5]*scale+offset};
 for(float f:xf)if(!std::isfinite(f))throw std::invalid_argument("GX viewport overflow");
 GDOverflowCheck(29);GDWrite_u8(0x10);GDWrite_u32(0x5101a);for(float f:xf)GDWrite_f32(f);
 retained=v;loaded=true;
}
}
extern "C" void GXSetViewportJitter(f32 left,f32 top,f32 width,f32 height,f32 nearz,f32 farz,u32 field){
 emit({left,top-(field==0?.5f:0.f),width,height,nearz,farz},zScale,zOffset);
}
extern "C" void GXSetViewport(f32 left,f32 top,f32 width,f32 height,f32 nearz,f32 farz){GXSetViewportJitter(left,top,width,height,nearz,farz,1);}
extern "C" void GXGetViewportv(f32* values){if(!values||!loaded)throw std::invalid_argument("GX viewport destination/state");std::copy(retained.begin(),retained.end(),values);}
extern "C" void GXSetZScaleOffset(f32 scale,f32 offset){
 float s=scale*16777215.f+1.f,o=offset*16777215.f;
 if(!std::isfinite(s)||!std::isfinite(o))throw std::invalid_argument("GX viewport Z scale/offset");
 if(loaded)emit(retained,s,o);zScale=s;zOffset=o;
}
