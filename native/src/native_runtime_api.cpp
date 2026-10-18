#define GALAXY_RUNTIME_EXPORTS
#include "native_runtime_api.h"
#include "native_collision_scene.hpp"
#include <Game/LiveActor/Binder.hpp>
#include <cmath>
#include <exception>
#include <string>
#include <stdexcept>
namespace {
NativeCollisionScene* owned=nullptr;
thread_local std::string error;
NativeCollisionScene& checked(void* p){if(!owned || p!=owned)throw std::invalid_argument("Invalid native runtime handle");return *owned;}
template<class F>int guarded(F operation){try{operation();error.clear();return 1;}catch(const std::exception& e){error=e.what();return 0;}catch(...){error="Unknown native runtime failure";return 0;}}
TVec3f vector(const float* v){if(!v || !std::isfinite(v[0]) || !std::isfinite(v[1]) || !std::isfinite(v[2]))throw std::invalid_argument("Invalid runtime vector");return TVec3f(v[0],v[1],v[2]);}
}
// Scalar SDK backend used by original collision methods within this module.
extern "C" float PSVECDotProduct(const Vec* a,const Vec* b){return a->x*b->x+a->y*b->y+a->z*b->z;}
uint32_t galaxy_runtime_version(){return 1;}
const char* galaxy_runtime_error(){return error.c_str();}
void* galaxy_runtime_create(int zones){NativeCollisionScene* result=nullptr;guarded([&]{if(owned)throw std::logic_error("Native runtime already exists");result=new NativeCollisionScene(zones);owned=result;});return result;}
int galaxy_runtime_destroy(void* p){return guarded([&]{auto& scene=checked(p);delete &scene;owned=nullptr;});}
int galaxy_runtime_tick(void* p){return guarded([&]{checked(p).tick();});}
int galaxy_runtime_load_terrain(void* p,const uint8_t* kcl,size_t ks,const uint8_t* metadata,size_t ms,const float* matrix,const char* name,int zone){return guarded([&]{
 auto& scene=checked(p);if(!kcl || !metadata || !matrix || !ks || !ms)throw std::invalid_argument("Missing runtime terrain data");
 TPos3f transform;for(int r=0;r<3;++r)for(int c=0;c<4;++c)transform.mMtx[r][c]=matrix[r*4+c];
 scene.loadTerrain(std::vector<uint8_t>(kcl,kcl+ks),std::vector<uint8_t>(metadata,metadata+ms),transform,name,zone);
});}
int galaxy_runtime_bind(void* p,const float* pos,const float* grav,const float* move,float radius,float* output,uint32_t* contacts){return guarded([&]{
 checked(p);if(!output || !contacts || !std::isfinite(radius) || radius<=0)throw std::invalid_argument("Invalid runtime Binder output or radius");
 auto position=vector(pos),gravity=vector(grav),movement=vector(move);
 if(gravity.squared()==0 || movement.length()>1000000)throw std::invalid_argument("Invalid runtime gravity or movement magnitude");
 gravity.normalize();Binder binder(nullptr,&position,&gravity,radius,0,0);auto result=binder.bind(movement);
 if(!std::isfinite(result.x)||!std::isfinite(result.y)||!std::isfinite(result.z))throw std::runtime_error("Nonfinite original Binder result");
 output[0]=result.x;output[1]=result.y;output[2]=result.z;*contacts=(binder.isBindedGround()?1u:0u)|(binder.isBindedWall()?2u:0u)|(binder.isBindedRoof()?4u:0u);
});}
