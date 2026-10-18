#pragma once
#include "Game/LiveActor/Nerve.hpp"
// Native sensor-host adapter with original actor nerve methods, not full LiveActor.
class NativeStatefulSensorHost:public LiveActor {
    std::unique_ptr<Spine> ownedSpine;bool updating=false;
public:
    explicit NativeStatefulSensorHost(const Nerve& initial){initNerve(&initial);ownedSpine.reset(mSpine);}
    NativeStatefulSensorHost(const NativeStatefulSensorHost&)=delete;
    NativeStatefulSensorHost& operator=(const NativeStatefulSensorHost&)=delete;
    void updateState(){assetCheck(!updating,"Recursive actor state update");assetCheck(mSpine==ownedSpine.get(),"Actor spine ownership changed");updating=true;try{mSpine->update();updating=false;}catch(...){updating=false;throw;}}
};
inline void liveActorNerveTests(){
 struct State:Nerve {mutable int executes=0,ends=0;mutable int observed=-99;void execute(Spine* spine)const override{++executes;observed=static_cast<LiveActor*>(spine->mExecutor)->getNerveStep();}void executeOnEnd(Spine*)const override{++ends;}};
 State idle,react;
 struct Actor:NativeStatefulSensorHost {const Nerve& reaction;Actor(const Nerve& initial,const Nerve& next):NativeStatefulSensorHost(initial),reaction(next){}bool receiveMsgPush(HitSensor*,HitSensor*)override{setNerve(&reaction);return true;}};
 Actor actor(idle,react);assetCheck(actor.isNerve(&idle)&&actor.getNerveStep()==0,"Original actor nerve initialization failed");actor.updateState();assetCheck(idle.executes==1&&idle.observed==0&&actor.getNerveStep()==1,"Original actor first-step timing failed");
 LiveActor senderHost;NativeSensorWorld world;auto& sender=world.create(1,1,1,senderHost);auto& receiver=world.create(0x46,1,1,actor);assetCheck(MR::sendMsgPush(&receiver,&sender)&&actor.isNerve(&react)&&actor.getNerveStep()==-1&&idle.ends==1,"Actor message-to-nerve transition failed");actor.updateState();assetCheck(react.executes==1&&react.observed==0&&actor.getNerveStep()==1,"Actor reaction first-step failed");actor.updateState();assetCheck(react.observed==1&&actor.getNerveStep()==2,"Actor reaction step advance failed");actor.setNerve(&idle);actor.updateState();assetCheck(react.ends==1&&idle.executes==2&&idle.observed==0,"Actor reaction return transition failed");
}
