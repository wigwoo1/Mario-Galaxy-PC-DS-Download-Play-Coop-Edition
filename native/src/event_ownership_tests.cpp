#include "Game/Player/PlayerEvent.hpp"
#include <cassert>
#include <cstdio>
#include <stdexcept>
// Test boundaries only: no live scene or player/audio services are invoked.
NameObj::NameObj(const char* name):mName(name),mFlag(0),mExecutorIdx(-1){}
NameObj::~NameObj()=default;
void NameObj::init(const JMapInfoIter&){} void NameObj::initAfterPlacement(){}
void NameObj::movement(){} void NameObj::draw()const{} void NameObj::calcAnim(){} void NameObj::calcViewAndEntry(){}
void EventSequencer::init(const JMapInfoIter&){}
namespace MR {
void startSoundPlayerJ(const char*){} void startBckPlayerJ(const char*){}
bool isMissLayoutAnimEnd(){return false;} void setPlayerSpot(f32,u32){}
void decPlayerLeft(){} void deactivateDefaultGameLayout(){} void startDownWipe(){}
bool isWipeActive(){return false;} void requestEndMissDemo(){}
}
struct OwnedEvent:EventSequence{
 static int alive,destroyed,calls;
 OwnedEvent():EventSequence(1){++alive;addEventOnTime("stop",static_cast<EventFunc1>(&OwnedEvent::stop),0);}
 ~OwnedEvent(){--alive;++destroyed;}
 void stop(u16 eventFrame,u16 frame){assert(eventFrame==0&&frame==0);++calls;stopSequence();}
};
int OwnedEvent::alive=0;int OwnedEvent::destroyed=0;int OwnedEvent::calls=0;
int main(){
 {
 EventSequencer sequence;
 sequence.addEventSequence<OwnedEvent>("owned");
 assert(OwnedEvent::alive==1);
 try{sequence.addEventSequence<OwnedEvent>("owned");assert(false);}catch(const std::exception&){}
 assert(OwnedEvent::alive==1&&OwnedEvent::destroyed==1);
 sequence.startEvent("owned");assert(sequence.running());
 NameObj* sceneObject=&sequence;sceneObject->movement();
 assert(!sequence.running()&&OwnedEvent::calls==1&&sequence.nextFrame()==1);
 sequence.startEvent("owned");sceneObject->movement();assert(OwnedEvent::calls==2);
 assert(OwnedEvent::alive==1);
 }
 assert(OwnedEvent::alive==0&&OwnedEvent::destroyed==2);
 {OwnedEvent borrowed;{EventSequencer sequence;sequence.registerSequence("borrowed",borrowed);}
 assert(OwnedEvent::alive==1);}
 assert(OwnedEvent::alive==0&&OwnedEvent::destroyed==3);
 std::puts("PASS event owned/borrowed lifetimes, duplicate rollback, original dispatch/restart and virtual scene movement");
}
