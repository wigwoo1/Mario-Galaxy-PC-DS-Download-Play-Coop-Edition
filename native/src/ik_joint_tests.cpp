#include <Game/Util/IKJoint.hpp>
#include <cmath>
#include <cstdio>
#include <stdexcept>
static void near(float a,float b){if(!std::isfinite(a)||std::abs(a-b)>0.003f)throw std::runtime_error("IK mismatch");}
int main(){try{
 IKJoint ik;ik.setRootBoneLength(3);ik.setMiddleBoneLength(4);
 near(ik.getMinLimitDistance(),1);near(ik.getMaxLimitDistance(),7);
 if(IKJoint::checkReachIKTarget(0.5f,3,4)!=1||IKJoint::checkReachIKTarget(8,3,4)!=2||IKJoint::checkReachIKTarget(5,3,4)!=0)throw std::runtime_error("IK reach classification");
 ik.updateByDistanceOnly(5);TVec3f end;ik.getEndJointPosition(&end);near(end.x,5);near(end.y,0);near(end.z,0);
 TVec3f middle;ik._60.getTrans(middle);near(middle.length(),3);near((end-middle).length(),4);
 ik.setFirstPose(TVec3f(1,0,0),TVec3f(0,1,0));TPos3f root;root.identity();
 if(ik.updateByLocalRootAndDirection(root,TVec3f(10,20,30),TVec3f(0,5,0))!=0)throw std::runtime_error("IK direction reach");
 ik.getEndJointPosition(&end);near(end.x,10);near(end.y,25);near(end.z,30);
 ik.updateByDistanceOnly(7);ik.getEndJointPosition(&end);near(end.x,7);near(end.y,0);
 std::puts("PASS original IK reach limits, bone lengths, target rotation and translated endpoint");return 0;
 }catch(const std::exception& e){std::puts(e.what());return 1;}}
