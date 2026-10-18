#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Gravity/PlanetGravityManager.hpp>
#include <Game/Gravity/PointGravity.hpp>
#include <Game/Gravity/GravityInfo.hpp>
#include <cmath>
#include <cstdio>
#include <stdexcept>
static void near(float a,float b){if(!std::isfinite(a)||std::abs(a-b)>0.003f)throw std::runtime_error("gravity manager mismatch");}
int main(){try{NameObjHolder names(4);SingletonHolder<NameObjRegister>::init();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(&names);PlanetGravityManager manager("gravity test");PointGravity a,b,low;
 a.mOrigPosition.set(100,0,0);b.mOrigPosition.set(0,100,0);low.mOrigPosition.set(0,0,100);
 a.updateIdentityMtx();b.updateIdentityMtx();low.updateIdentityMtx();a.setPriority(5);b.setPriority(5);low.setPriority(0);
 manager.registerGravity(&low);manager.registerGravity(&a);manager.registerGravity(&b);
 TVec3f result;GravityInfo info;
 if(!manager.calcTotalGravityVector(&result,&info,TVec3f(0,0,0),GRAVITY_TYPE_NORMAL,0))throw std::runtime_error("gravity missing");near(result.x,0.70710678f);near(result.y,0.70710678f);near(result.z,0);if(info.mLargestPriority!=5)throw std::runtime_error("priority selection");
 b.mActivated=false;manager.calcTotalGravityVector(&result,&info,TVec3f(0,0,0),1,0);near(result.x,1);near(result.y,0);
 a.mAppeared=false;manager.calcTotalGravityVector(&result,&info,TVec3f(0,0,0),1,0);near(result.z,1);
 a.mAppeared=true;b.mActivated=true;
 // Two distinct 64-bit host tokens with identical low 32 bits must remain distinct.
 a.mHost=reinterpret_cast<const void*>(std::uintptr_t(0x100000001ULL));b.mHost=reinterpret_cast<const void*>(std::uintptr_t(0x200000001ULL));
 manager.calcTotalGravityVector(&result,&info,TVec3f(0,0,0),1,0x100000001ULL);near(result.x,0);near(result.y,1);
 a.mGravityType=GRAVITY_TYPE_SHADOW;b.mValidFollower=false;
 manager.calcTotalGravityVector(&result,&info,TVec3f(0,0,0),1,0);near(result.z,1);
 low.mActivated=false;if(manager.calcTotalGravityVector(&result,&info,TVec3f(0,0,0),1,0))throw std::runtime_error("inactive gravity selected");near(result.length(),0);
 names.clearArray();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);
 std::puts("PASS original gravity priority blending, activation/type filtering and 64-bit host exclusion");return 0;
 }catch(const std::exception& e){std::puts(e.what());return 1;}}

