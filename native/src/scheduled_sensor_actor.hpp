#pragma once
#include "stateful_sensor_host.hpp"
#include "native_scene.hpp"
// Native scheduling bridge, not the original LiveActor inheritance/layout or movement body.
class NativeScheduledSensorActor:public NameObj,public NativeStatefulSensorHost {
public:
    NativeScheduledSensorActor(const char* name,const Nerve& initial):NameObj(name),NativeStatefulSensorHost(initial){}
    void movement()override{if(!isDead()&&!isClipped()){updateState();if(!isDead())control();}}
    virtual void control(){}
};
inline void scheduledSensorActorTests(){
 struct State:Nerve {mutable int calls=0,lastStep=-99,ends=0;void execute(Spine* spine)const override{++calls;lastStep=static_cast<LiveActor*>(spine->mExecutor)->getNerveStep();}void executeOnEnd(Spine*)const override{++ends;}};State idle,reaction;
 struct Actor:NativeScheduledSensorActor {const Nerve& reaction;int controls=0;bool dead=false,clipped=false;Actor(const Nerve& initial,const Nerve& next):NativeScheduledSensorActor("scheduled sensor actor",initial),reaction(next){}bool receiveMsgPush(HitSensor*,HitSensor*)override{setNerve(&reaction);return true;}void control()override{++controls;}bool isDead()const override{return dead;}bool isClipped()const override{return clipped;}};
 NativeScene scene;auto& actor=scene.create<Actor>(idle,reaction);LiveActor sender;NativeSensorWorld world;auto& source=world.create(1,1,1,sender);auto& keeper=world.createKeeperForType(0x46,1);auto& target=keeper.add("body",0x46,1,1,actor,{});keeper.validate();
 scene.tick();assetCheck(idle.calls==1&&idle.lastStep==0&&actor.controls==1,"Scheduled actor first frame failed");MR::sendMsgPush(&target,&source);scene.tick();assetCheck(reaction.calls==1&&reaction.lastStep==0&&idle.ends==1&&actor.controls==2,"Scheduled actor message reaction failed");actor.requestSuspend();scene.tick();assetCheck(reaction.calls==1&&actor.controls==2,"Suspended scheduled actor updated");actor.requestResume();scene.tick();assetCheck(reaction.calls==2&&reaction.lastStep==1,"Resumed scheduled actor lost state timing");actor.dead=true;scene.tick();assetCheck(reaction.calls==2,"Dead scheduled actor state updated");actor.dead=false;actor.clipped=true;scene.tick();assetCheck(reaction.calls==2,"Clipped scheduled actor state updated");actor.clipped=false;scene.tick();assetCheck(reaction.calls==3&&reaction.lastStep==2&&actor.controls==4,"Scheduled actor state did not resume after visibility change");
}
