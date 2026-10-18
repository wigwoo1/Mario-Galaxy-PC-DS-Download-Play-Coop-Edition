#include <Game/LiveActor/HitSensor.hpp>
#include <Game/LiveActor/SensorHitChecker.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
#include <Game/Scene/SceneFunction.hpp>
#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Util/ActorSensorUtil.hpp>
#include <cassert>
#include <cstdio>
namespace {SceneObjHolder* scene=nullptr;const LiveActor* clipped=nullptr;int connections=0;}
namespace MR {
 SceneObjHolder* getSceneObjHolder(){return scene;}
 bool isClipped(const LiveActor* actor){return actor==clipped;}
 void connectToScene(NameObj*,int movement,int animation,int buffer,int draw){assert(movement==MovementType_SensorHitChecker&&animation==CalcAnimType_None&&buffer==DrawBufferType_None&&draw==DrawType_None);++connections;}
}
NameObj* SceneObjHolder::newEachObj(int id){assert(id==SceneObj_SensorHitChecker);return new SensorHitChecker("original sensors");}
int main(){
 NameObjHolder names(8);SingletonHolder<NameObjRegister>::init();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(&names);
 SceneObjHolder holder;scene=&holder;auto* ownedChecker=static_cast<SensorHitChecker*>(holder.create(SceneObj_SensorHitChecker));auto& checker=*ownedChecker;
 assert(connections==1&&holder.create(SceneObj_SensorHitChecker)==ownedChecker&&connections==1);
 // Identity-only opaque actor pointers: original checker compares them; fixture clipping never dereferences them.
 int identities[3];auto* hostA=reinterpret_cast<LiveActor*>(&identities[0]);auto* hostB=reinterpret_cast<LiveActor*>(&identities[1]);
 HitSensor player(ATYPE_PLAYER,4,10,hostA),enemy(ATYPE_ENEMY,4,20,hostB),eye(ATYPE_EYE,4,1,hostB);
 assert(player.mSensorGroup==checker.mPlayerGroup&&enemy.mSensorGroup==checker.mCharacterGroup&&eye.mSensorGroup==checker.mEyeGroup);
 player.validateBySystem();enemy.validateBySystem();
 enemy.mPosition.set(30,0,0);checker.movement();assert(player.mSensorCount==0&&enemy.mSensorCount==0);
 enemy.mPosition.set(29,0,0);checker.movement();assert(player.mSensorCount==1&&enemy.mSensorCount==1&&player.mSensors[0]==&enemy&&enemy.mSensors[0]==&player);
 checker.movement();assert(player.mSensorCount==1&&enemy.mSensorCount==1);
 clipped=hostB;checker.movement();assert(player.mSensorCount==0&&enemy.mSensorCount==0);clipped=nullptr;
 enemy.invalidate();checker.movement();assert(player.mSensorCount==0);enemy.validate();
 enemy.mHost=hostA;checker.movement();assert(player.mSensorCount==0&&enemy.mSensorCount==0);enemy.mHost=hostB;
 enemy.invalidate();eye.validateBySystem();eye.mPosition.set(0,0,0);checker.movement();assert(player.mSensorCount==0&&eye.mSensorCount==1&&eye.mSensors[0]==&player);
 player.invalidateBySystem();enemy.invalidateBySystem();eye.invalidateBySystem();
 delete[] player.mSensors;delete[] enemy.mSensors;delete[] eye.mSensors;
 SensorGroup* groups[]={checker.mPlayerGroup,checker.mRideGroup,checker.mEyeGroup,checker.mSimpleGroup,checker.mMapObjGroup,checker.mCharacterGroup};for(auto* group:groups){delete[] group->mSensors;delete group;}
 delete ownedChecker;names.clearArray();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);
 std::puts("PASS original sensor scene routing and movement: tangent exclusion, overlap, frame clear, clipping, validity, same host and eye asymmetry");
}
