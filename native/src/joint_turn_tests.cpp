#include <Game/Util/MathUtil.hpp>
#include <Game/Util/MtxUtil.hpp>
#include <cmath>
#include <cstdio>
#include <stdexcept>
static void near(float a,float b){if(!std::isfinite(a)||std::abs(a-b)>0.01f)throw std::runtime_error("joint turn mismatch");}
int main(){try{MR::initAcosTable();TQuat4f q(0,0,0,1);TVec3f x;
 MR::turnQuatXDirRad(&q,q,TVec3f(0,1,0),0.5235987756f);q.getXDir(x);near(x.x,0.8660254f);near(x.y,0.5f);near(q.squared(),1);
 TPos3f m;m.identity();m.setTrans(10,20,30);MR::turnMtxToXDirDegree(&m,TVec3f(0,1,0),30);m.getXDir(x);near(x.x,0.8660254f);near(x.y,0.5f);near(m.mMtx[0][3],10);near(m.mMtx[1][3],20);near(m.mMtx[2][3],30);
 m.identity();MR::turnMtxToLocalDirDegree(&m,TVec3f(0,1,0),TVec3f(0,0,1),45);m.getYDir(x);near(x.y,0.70710678f);near(x.z,0.70710678f);
 m.identity();MR::turnMtxToXDirDegree(&m,TVec3f(0,1,0),180);m.getXDir(x);near(x.x,0);near(x.y,1);
 std::puts("PASS original quaternion and matrix capped turns, local axis and translation preservation; random branch not exercised");return 0;
 }catch(const std::exception& e){std::puts(e.what());return 1;}}
