#include <Game/Map/StageSwitch.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Util/JMapIdInfo.hpp>
#include <Game/Map/SleepController.hpp>
#include <Game/Map/ActorAppearSwitchListener.hpp>
#include <cstring>
#include <JSystem/JGadget/hashcode.hpp>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cstdio>
static SceneObjHolder* fixtureScene=nullptr;static int fixtureZone=0;
namespace MR {SceneObjHolder* getSceneObjHolder(){assert(fixtureScene);return fixtureScene;}s32 getPlacedZoneId(const JMapInfoIter&){return fixtureZone;}}
NameObj* SceneObjHolder::newEachObj(int id){assert(id==SceneObj_StageSwitchContainer);return new StageSwitchContainer();}
struct SleepListener:SwitchEventListener{int on=0,off=0;void listenSwitchOnEvent()override{++on;}void listenSwitchOffEvent()override{++off;}};
int main(){NameObjHolder names(8);SingletonHolder<NameObjRegister>::init();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(&names);
SceneObjHolder scene;fixtureScene=&scene;auto* container=static_cast<StageSwitchContainer*>(scene.create(SceneObj_StageSwitchContainer));assert(scene.create(SceneObj_StageSwitchContainer)==container);
SwitchIdInfo local(31,JMapInfoIter());container->createAndAddZone(local);fixtureZone=1;SwitchIdInfo other(31,JMapInfoIter());container->createAndAddZone(other);SwitchIdInfo global(1031,JMapInfoIter());
StageSwitchFunction::onSwitchBySwitchIdInfo(local);StageSwitchFunction::onSwitchBySwitchIdInfo(local);
assert(StageSwitchFunction::isOnSwitchBySwitchIdInfo(local)&&!StageSwitchFunction::isOnSwitchBySwitchIdInfo(other)&&!StageSwitchFunction::isOnSwitchBySwitchIdInfo(global));
StageSwitchFunction::onSwitchBySwitchIdInfo(global);StageSwitchFunction::offSwitchBySwitchIdInfo(local);StageSwitchFunction::offSwitchBySwitchIdInfo(local);
assert(!StageSwitchFunction::isOnSwitchBySwitchIdInfo(local)&&StageSwitchFunction::isOnSwitchBySwitchIdInfo(global)&&!StageSwitchFunction::isOnSwitchBySwitchIdInfo(other));
std::vector<std::uint8_t> table(40,0);
auto putBE=[&](int offset,std::uint32_t value){for(int byte=0;byte<4;byte++)table[offset+byte]=std::uint8_t(value>>(24-byte*8));};
putBE(0,3);putBE(4,1);putBE(8,28);putBE(12,4);
putBE(16,JGadget::getHashCode("SW_SLEEP"));putBE(20,0xffffffff);
putBE(28,5);putBE(32,1005);putBE(36,0xffffffff);
JMapInfo placement;assert(placement.attachNative(table));
for(int row=0;row<3;row++){s32 decoded=0;assert(placement.getValue(row,"SW_SLEEP",&decoded));assert(decoded==(row==0?5:row==1?1005:-1));}
assert(StageSwitchFunction::createSwitchIdInfo("missing",JMapInfoIter(&placement,0),false)==nullptr);
assert(StageSwitchFunction::createSwitchIdInfo("SW_SLEEP",JMapInfoIter(&placement,2),false)==nullptr);
auto* globalSleep=StageSwitchFunction::createSwitchIdInfo("SW_SLEEP",JMapInfoIter(&placement,1),false);assert(globalSleep&&globalSleep->mIsGlobal&&globalSleep->getSwitchNo()==5);delete globalSleep->mIDInfo;delete globalSleep;
SleepListener listener;SleepController sleeper(JMapInfoIter(&placement,0),&listener);sleeper.initSync();assert(listener.off==1&&listener.on==0);
StageSwitchFunction::onSwitchBySwitchIdInfo(*sleeper.mSwitchIdInfo);sleeper.update();assert(listener.on==1&&listener.off==1);
for(int frame=0;frame<120;frame++)sleeper.update();assert(listener.on==1&&listener.off==1);
StageSwitchFunction::offSwitchBySwitchIdInfo(*sleeper.mSwitchIdInfo);sleeper.update();assert(listener.off==2&&listener.on==1);
assert(StageSwitchFunction::isOnSwitchBySwitchIdInfo(global));
delete sleeper.mSwitchIdInfo->mIDInfo;delete sleeper.mSwitchIdInfo;
delete local.mIDInfo;delete other.mIDInfo;delete global.mIDInfo;delete container;fixtureScene=nullptr;names.clearArray();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);
std::puts("PASS big-endian placement lookup, original scene cache/routing and sleep events with120unchanged frames");}
