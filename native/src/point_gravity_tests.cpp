#include <Game/Gravity/PointGravity.hpp>
#include <cmath>
#include <cstdio>
#include <stdexcept>
static void near(float a,float b){if(!std::isfinite(a)||std::abs(a-b)>0.01f)throw std::runtime_error("point gravity mismatch");}
int main(){try{PointGravity g;g.updateIdentityMtx();TVec3f v;
 if(!g.calcGravity(&v,TVec3f(100,0,0)))throw std::runtime_error("gravity missing");near(v.x,-400);near(v.y,0);near(v.z,0);
 g.mIsInverse=true;g.calcGravity(&v,TVec3f(100,0,0));near(v.x,400);g.mIsInverse=false;
 g.mRange=100;v.set(7,8,9);if(g.calcGravity(&v,TVec3f(100,0,0)))throw std::runtime_error("range boundary accepted");near(v.x,7);near(v.y,8);near(v.z,9);
 if(!g.calcGravity(&v,TVec3f(99,0,0)))throw std::runtime_error("inside range rejected");
 g.mRange=-1;g.mOrigPosition.set(1,2,3);TPos3f transform;transform.identity();transform.setTrans(10,20,30);g.updateMtx(transform);
 g.calcGravity(&v,TVec3f(111,22,33));near(v.x,-400);near(v.y,0);near(v.z,0);
 g.calcGravity(&v,TVec3f(11,22,33));near(v.x,0);near(v.y,0);near(v.z,0);
 g.mDistant=50;g.calcGravity(&v,TVec3f(111,22,33));near(v.x,-1600);
 std::puts("PASS original point gravity inverse-square magnitude, inversion, exclusive range, transformed center and distance offset");return 0;
 }catch(const std::exception& e){std::puts(e.what());return 1;}}
