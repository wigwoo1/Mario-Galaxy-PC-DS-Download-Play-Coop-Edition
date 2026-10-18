#include <Game/Ride/SwingRopePoint.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
static void near(float a,float b){assert(std::fabs(a-b)<.001f);}
int main(){
 SwingRopePoint point(TVec3f(0,-5,0));
 near(point.mVelocity.squared(),0);near(point.mSide.x,1);near(point.mUp.y,1);near(point.mFront.z,1);
 point.addAccel(TVec3f(0,-2,0));assert(point.restrict(TVec3f(0,0,0),5,nullptr));
 point.updatePos(.5f);near(point.mPosition.y,-5);near(point.mVelocity.y,0);
 point.mPosition.set(1,0,0);point.mVelocity.set(0,0,0);
 assert(!point.restrict(TVec3f(0,0,0),5,nullptr));near(point.mPosition.x,1);
 point.strain(TVec3f(0,0,0),5);point.updatePos(.5f);near(point.mPosition.x,5);near(point.mVelocity.x,2);
 point.stop();near(point.mVelocity.squared(),0);
 point.setInfo(TVec3f(2,3,4),TVec3f(0,0,0),TVec3f(2,3,4),5);
 near(point.mPosition.x,2);near(point.mPosition.y,-2);near(point.mPosition.z,4);
 near(point.mUp.y,1);
 for(unsigned tick=0;tick<240;++tick){
   TVec3f anchor(float(tick)*.01f,0,0),anchorVelocity(.01f,0,0);
   point.addAccel(TVec3f(.01f,-.15f,.02f));
   point.restrict(anchor,5,&anchorVelocity);point.updatePosAndAxis(TVec3f(0,0,1),.97f);
   TVec3f relative=point.mPosition-anchor-anchorVelocity;
   assert(relative.squared()<=25.01f);
   near(point.mSide.squared(),1);near(point.mFront.squared(),1);near(point.mUp.squared(),1);
   near(point.mSide.dot(point.mUp),0);near(point.mFront.dot(point.mUp),0);
 }
 point.setAndUpdatePosAndAxis(TVec3f(7,8,9),TVec3f(0,1,0),TVec3f(0,1,0));
 near(point.mPosition.x,7);near(point.mVelocity.squared(),0);
 std::puts("PASS original SwingRopePoint: length constraints, moving anchor, damping, stop and 240 orthonormal axis updates");
}
