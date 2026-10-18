#include <Game/Map/SleepController.hpp>
#include <Game/Map/StageSwitch.hpp>
#include <Game/Map/ActorAppearSwitchListener.hpp>
#include <Game/Util/JMapIdInfo.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
static ZoneSwitch fixtureSwitches;
namespace MR{s32 getPlacedZoneId(const JMapInfoIter&){return 0;}}
SwitchIdInfo* StageSwitchFunction::createSwitchIdInfo(const char* name,const JMapInfoIter& iter,bool unused){assert(!std::strcmp(name,"SW_SLEEP")&&!unused);return new SwitchIdInfo(5,iter);}
bool StageSwitchFunction::isOnSwitchBySwitchIdInfo(const SwitchIdInfo& info){assert(info.getSwitchNo()==5);return fixtureSwitches.get(5);}
struct Listener:SwitchEventListener{int on=0,off=0;void listenSwitchOnEvent()override{++on;}void listenSwitchOffEvent()override{++off;}};
int main(){
 Listener listener;SleepController controller(JMapInfoIter(),&listener);
 controller.initSync();assert(listener.off==1&&listener.on==0);
 for(int i=0;i<60;i++)controller.update();assert(listener.off==1&&listener.on==0);
 fixtureSwitches.set(5,true);controller.update();assert(listener.on==1);
 for(int i=0;i<60;i++)controller.update();assert(listener.on==1&&listener.off==1);
 fixtureSwitches.set(5,false);controller.update();assert(listener.off==2);
 delete controller.mSwitchIdInfo->mIDInfo;delete controller.mSwitchIdInfo;
 Listener initiallyOn;fixtureSwitches.set(5,true);SleepController second(JMapInfoIter(),&initiallyOn);
 second.initSync();assert(initiallyOn.on==1);second.update();assert(initiallyOn.on==2);second.update();assert(initiallyOn.on==2);
 delete second.mSwitchIdInfo->mIDInfo;delete second.mSwitchIdInfo;
 std::puts("PASS original sleep controller: initial sync, unchanged frames, on/off transitions and original initial-on repeat");
}
