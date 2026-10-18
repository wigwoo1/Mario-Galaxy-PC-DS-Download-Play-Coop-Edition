#include <Game/Util/MathUtil.hpp>
#include <cmath>
#include <cstdio>
#include <stdexcept>
static void near(float a,float b){if(!std::isfinite(a)||std::abs(a-b)>0.01f){std::printf("actual %.9f expected %.9f\n",a,b);throw std::runtime_error("direction blend mismatch");}}
int main(){try{MR::initAcosTable();TVec3f v;
 if(!MR::vecBlendSphere(TVec3f(2,0,0),TVec3f(0,4,0),&v,0.5f))throw std::runtime_error("blend failed");near(v.length(),3);near(v.y/v.x,2);
 MR::vecBlendSphere(TVec3f(2,0,0),TVec3f(0,4,0),&v,0);near(v.x,2);near(v.y,0);
 MR::vecBlendSphere(TVec3f(2,0,0),TVec3f(0,4,0),&v,1);near(v.x,0);near(v.y,4);
 MR::vecBlendSphere(TVec3f(2,0,0),TVec3f(4,0,0),&v,0.5f);near(v.x,3);near(v.y,0);
 v.set(7,8,9);if(MR::vecBlendSphere(TVec3f(1,0,0),TVec3f(-1,0,0),&v,0.5f))throw std::runtime_error("opposite should fail");near(v.x,7);near(v.y,8);near(v.z,9);
 MR::vecBlendSphere(TVec3f(0,0,0),TVec3f(2,0,0),&v,0.5f);near(v.x,1);near(v.y,0);
 const float endpoint=MR::acosEx(1);if(endpoint<0||endpoint>0.013f)throw std::runtime_error("acos endpoint table range");near(MR::acosEx(-1),MR::pi()-endpoint);
 std::puts("PASS original spherical blend endpoints, interpolated length, parallel/zero/opposite cases and acos table");return 0;
 }catch(const std::exception& e){std::puts(e.what());return 1;}}


