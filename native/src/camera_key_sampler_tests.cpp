#include <Native/OwnedCameraAnimation.hpp>
#include <Game/Camera/CameraAnim.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 KeyCamAnmDataAccessor sampler;float v3[]={10,20,30,40,50,30};
 sampler.mValues=v3;
 assert(sampler.get(0,0,2,0)==20);assert(sampler.get(10,0,2,0)==20);
 assert(sampler.get(40,0,2,0)==50);assert(sampler.get(100,0,2,0)==50);
 for(int t=11;t<40;++t)assert(std::fabs(sampler.get(float(t),0,2,0)-(10+t))<0.0001f);
 float v4[]={10,20,99,30,40,50,30,99};sampler.mValues=v4;
 assert(sampler.get(-10,0,2,1)==20);assert(sampler.get(40,0,2,1)==50);
 assert(std::fabs(sampler.get(25,0,2,1)-35)<0.0001f);
 assert(sampler.get(100,1,1,0)==20);
 std::vector<uint8_t> bytes(156);std::memcpy(bytes.data(),"ANDOCKAN",8);
 auto put=[&](size_t p,uint32_t v){for(int i=3;i>=0;--i){bytes[p+i]=v&255;v>>=8;}};
 put(8,1);put(24,31);put(28,96);put(128,24);
 for(int i=0;i<8;++i){put(32+i*12,2);put(36+i*12,0);put(40+i*12,0);}
 put(132,0);put(136,0x41a00000);put(140,0x41f00000);put(144,0x41f00000);put(148,0x42480000);put(152,0x41f00000);
 Native::OwnedCameraAnimation owned;assert(owned.load(bytes.data(),bytes.size()));
 CamAnmDataAccessor frame;KeyCamAnmDataAccessor key;
 auto* active=owned.bind(frame,key);assert(active==&key&&owned.frames()==31);
 TVec3f pos;active->getPos(&pos,15);assert(std::fabs(pos.x-35)<0.0001f&&pos.x==pos.y&&pos.y==pos.z);
 assert(active->getFovy(30)==50);assert(active->getTwist(0)==20);
 bytes.clear();assert(active->getFovy(15)==35);
 assert(!owned.load(nullptr,0));assert(active->getFovy(15)==35);
 bytes.resize(108);std::memcpy(bytes.data(),"ANDOCANM",8);put(8,1);put(24,2);put(28,64);put(96,8);
 for(int i=0;i<8;++i){put(32+i*8,2);put(36+i*8,0);}
 put(100,0x41a00000);put(104,0x42480000);
 assert(owned.load(bytes.data(),bytes.size()));active=owned.bind(frame,key);assert(active==&frame);
 assert(active->getFovy(0)==20&&active->getFovy(0.5f)==35&&active->getFovy(1)==50);
 active->getWatchPos(&pos,0.5f);assert(pos.x==35&&pos.y==35&&pos.z==35);
 std::puts("PASS original camera Hermite sampling, first/last clamping, both tangent layouts and constant channel");
}


