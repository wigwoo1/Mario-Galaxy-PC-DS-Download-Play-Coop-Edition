#include <JSystem/JGeometry/TMatrix.hpp>
#include <math_types.hpp>
#include <Game/Util/FixedPosition.hpp>
#include <cmath>
#include <cstdio>
#include <stdexcept>
static void check(const TPos3f& m,const float (&e)[3][4]){for(int r=0;r<3;r++)for(int c=0;c<4;c++)if(std::abs(m.mMtx[r][c]-e[r][c])>0.002f)throw std::runtime_error("FixedPosition transform mismatch");}
int main(){try{
 Mtx parent={{0,-3,0,10},{2,0,0,20},{0,0,4,30}};
 FixedPosition pos(parent,TVec3f(1,2,3),TVec3f(0,0,0));pos.calc();
 const float normalized[3][4]={{0,-1,0,4},{1,0,0,22},{0,0,1,42}};check(pos.mMtx,normalized);
 pos.mNormalizeScale=false;pos.calc();const float scaled[3][4]={{0,-3,0,4},{2,0,0,22},{0,0,4,42}};check(pos.mMtx,scaled);
 pos.setBaseMtx(nullptr);pos.setLocalTrans(TVec3f(7,8,9));pos.mLocalRotate.set(0,0,90);pos.calc();
 const float local[3][4]={{0,-1,0,7},{1,0,0,8},{0,0,1,9}};check(pos.mMtx,local);
 pos.setBaseMtx(parent);pos.mNormalizeScale=true;pos.calc();
 const float composed[3][4]={{-1,0,0,-14},{0,-1,0,34},{0,0,1,66}};check(pos.mMtx,composed);
 std::puts("PASS original FixedPosition parent composition, local setters, scale normalization and detached rotation");return 0;
 }catch(const std::exception& e){std::puts(e.what());return 1;}}
