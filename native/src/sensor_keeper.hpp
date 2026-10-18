#include "actor_state_keeper.hpp"
#pragma once
#include <list>
#include "Game/LiveActor/Spine.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "sensor_runtime.hpp"
#include "Game/LiveActor/HitSensorKeeper.hpp"
#include "Game/LiveActor/HitSensorInfo.hpp"
// Owns one bounded registration group. Original type-category routing is not implemented.
class NativeSensorKeeper {
    static int checkedCapacity(int n){assetCheck(n>0&&n<=2048,"Invalid native sensor keeper capacity");return n;}
    SensorGroup group;
    SensorGroup* registrationGroup;
    HitSensorKeeper keeper;
    std::list<std::string> names;
    void check(const char* name,float radius){assetCheck(name&&*name&&std::isfinite(radius)&&radius>=0,"Invalid named sensor");assetCheck(keeper.mSensorInfosSize<keeper.mSensorCount,"Native sensor keeper capacity exceeded");for(const auto& n:names)assetCheck(n!=name,"Duplicate native sensor name");}
public:
    explicit NativeSensorKeeper(int capacity):group(checkedCapacity(capacity),"native keeper"),registrationGroup(&group),keeper(capacity){assetCheck(capacity>0&&capacity<=2048,"Invalid native sensor keeper capacity");}
    NativeSensorKeeper(int capacity,SensorGroup& external):group(checkedCapacity(capacity),"unused keeper group"),registrationGroup(&external),keeper(capacity){}
    NativeSensorKeeper(const NativeSensorKeeper&)=delete;
    NativeSensorKeeper& operator=(const NativeSensorKeeper&)=delete;
    ~NativeSensorKeeper(){keeper.invalidateBySystem();for(int i=0;i<keeper.mSensorInfosSize;++i){auto info=keeper.mSensorInfos[i];delete[] info->mSensor->mSensors;delete info->mSensor;delete info;}delete[] keeper.mSensorInfos;delete[] group.mSensors;}
    HitSensor& add(const char* name,u32 type,u16 contacts,float radius,LiveActor& host,const TVec3f& offset){check(name,radius);names.emplace_back(name);NativeSensorGroupScope scope(*registrationGroup);return *keeper.add(names.back().c_str(),type,contacts,radius,&host,offset);}
    HitSensor& addPos(const char* name,u32 type,u16 contacts,float radius,LiveActor& host,const TVec3f& position,const TVec3f& offset){check(name,radius);names.emplace_back(name);NativeSensorGroupScope scope(*registrationGroup);return *keeper.addPos(names.back().c_str(),type,contacts,radius,&host,&position,offset);}
    HitSensor& addMtx(const char* name,u32 type,u16 contacts,float radius,LiveActor& host,MtxPtr matrix,const TVec3f& offset){assetCheck(matrix!=nullptr,"Missing sensor matrix");check(name,radius);names.emplace_back(name);NativeSensorGroupScope scope(*registrationGroup);return *keeper.addMtx(names.back().c_str(),type,contacts,radius,&host,matrix,offset);}
    HitSensor& addCallback(const char* name,u32 type,u16 contacts,float radius,LiveActor& host){check(name,radius);names.emplace_back(name);NativeSensorGroupScope scope(*registrationGroup);return *keeper.addCallback(names.back().c_str(),type,contacts,radius,&host);}
    int reservedCapacity()const{return keeper.mSensorCount;}
    SensorGroup& registeredGroup()const{return *registrationGroup;}
    HitSensor* get(const char* name)const{assetCheck(name!=nullptr,"Missing sensor lookup name");return keeper.getSensor(name);}
    void update(){keeper.update();for(int i=0;i<keeper.mSensorInfosSize;++i){auto sensor=keeper.mSensorInfos[i]->mSensor;assetCheck(std::isfinite(sensor->mPosition.x)&&std::isfinite(sensor->mPosition.y)&&std::isfinite(sensor->mPosition.z)&&std::isfinite(sensor->mRadius)&&sensor->mRadius>=0,"Invalid named sensor geometry");}}
    void dispatch(){keeper.doObjCol();}
    void clear(){keeper.clear();}
    void validate(){keeper.validateBySystem();keeper.validate();}
    void invalidate(){keeper.invalidate();}
    void validateBySystem(){keeper.validateBySystem();}
    void invalidateBySystem(){keeper.clear();keeper.invalidateBySystem();}
};
// Native subset of original LiveActor appearance/death sensor policy.
class NativeSensorLifecycleHost:public LiveActor {
    NativeSensorKeeper* sensors;std::unique_ptr<Spine> ownedSpine;std::vector<std::unique_ptr<ActorStateBaseInterface>> ownedStateInterfaces;std::unique_ptr<NativeActorStateKeeper> ownedStates;
public:
    explicit NativeSensorLifecycleHost(NativeSensorKeeper& keeper):sensors(&keeper){}
    NativeSensorLifecycleHost(const NativeSensorLifecycleHost&)=delete;NativeSensorLifecycleHost& operator=(const NativeSensorLifecycleHost&)=delete;
    ~NativeSensorLifecycleHost()override{mSpine=nullptr;}
    void initNerve(const Nerve* nerve){assetCheck(nerve&&mSpine==nullptr,"Invalid or repeated lifecycle nerve initialization");LiveActor::initNerve(nerve);ownedSpine.reset(mSpine);}
    void initStateKeeper(int capacity){assetCheck(mSpine&&!ownedStates,"Invalid lifecycle state keeper initialization");auto states=std::make_unique<NativeActorStateKeeper>(capacity);states->attachTo(*mSpine);ownedStates=std::move(states);}
    template<class State,class... Args> State& createState(const Nerve* nerve,const char* name,Args&&... args){assetCheck(bool(ownedStates),"Missing lifecycle state keeper");ownedStateInterfaces.reserve(ownedStateInterfaces.size()+1);auto state=std::make_unique<State>(std::forward<Args>(args)...);initState(state.get(),nerve,name);auto& result=*state;ownedStateInterfaces.push_back(std::move(state));return result;}
    void initState(ActorStateBaseInterface* state,const Nerve* nerve,const char* name){assetCheck(bool(ownedStates),"Missing lifecycle state keeper");ownedStates->initState(state,nerve,name);}
    void addState(ActorStateBaseInterface* state,const Nerve* nerve,const char* name){assetCheck(bool(ownedStates),"Missing lifecycle state keeper");ownedStates->addState(state,nerve,name);}
    void appear()override{if(sensors)sensors->validateBySystem();LiveActor::appear();if(sensors)sensors->update();}
    void kill()override{if(sensors)sensors->invalidateBySystem();LiveActor::kill();}
    NativeSensorKeeper& sensorKeeper(){assetCheck(sensors!=nullptr,"Lifecycle host has no attached keeper");return *sensors;}
    void detachKeeper(){sensors=nullptr;}
    void dispatchSensors(){if(sensors)sensors->dispatch();}
    void tickNerve(){if(!isDead()&&mSpine)mSpine->update();}
    bool updateStateAndNextNerve(const Nerve* next){assetCheck(ownedStates&&next,"Invalid lifecycle state update");if(ownedStates->updateCurrentState()){setNerve(next);return true;}return false;}
};
inline void sensorKeeperTests(){
 struct Host:LiveActor {int attacks=0,callbacks=0;bool receiveMessage(u32,HitSensor*,HitSensor*)override{return false;}void attackSensor(HitSensor*,HitSensor*)override{++attacks;}void updateHitSensor(HitSensor* sensor)override{++callbacks;sensor->mPosition=mPosition;}};
 Host a,b;a.mPosition={10,20,30};NativeSensorKeeper keeper(4);std::string temporary="body";auto& body=keeper.add(temporary.c_str(),1,2,5,a,{1,2,3});temporary="changed";assetCheck(keeper.get("anything")==&body,"Original single-sensor lookup shortcut changed");
 TVec3f external(100,200,300);auto& pos=keeper.addPos("position",2,2,5,b,external,{0,0,0});Mtx matrix={{1,0,0,9},{0,1,0,8},{0,0,1,7}};auto& transformed=keeper.addMtx("matrix",2,1,1,b,matrix,{2,3,4});auto& callback=keeper.addCallback("callback",2,1,1,b);
 assetCheck(keeper.get("body")==&body&&keeper.get("position")==&pos&&keeper.get("unknown")==nullptr,"Original named sensor lookup failed");assetCheck(body.mPosition.x==11&&transformed.mPosition.x==11&&b.callbacks==1,"Keeper immediate position update failed");a.mPosition.x=40;external.x=120;matrix[0][3]=20;keeper.update();assetCheck(body.mPosition.x==41&&pos.mPosition.x==120&&transformed.mPosition.x==22&&b.callbacks==2,"Keeper bulk position update failed");keeper.validate();assetCheck(body.isValid()&&callback.isValid(),"Keeper validation failed");body.addHitSensor(&pos);keeper.dispatch();assetCheck(a.attacks==1,"Keeper contact dispatch failed");keeper.clear();assetCheck(body.mSensorCount==0,"Keeper contact clear failed");keeper.invalidate();assetCheck(!body.isValid(),"Keeper invalidation failed");keeper.validate();assetCheck(body.isValid(),"Keeper revalidation failed");bool rejected=false;try{keeper.add("overflow",1,1,1,a,{});}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Keeper capacity not enforced");
 NativeSensorKeeper duplicates(2);duplicates.add("body",1,1,1,a,{});rejected=false;try{duplicates.add("body",1,1,1,a,{});}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Duplicate sensor name not rejected");
}




inline void nativeLifecycleNerveTests(){struct Probe:Nerve{int* count;explicit Probe(int& value):count(&value){}void execute(Spine*)const override{++*count;}};int updates=0;Probe nerve(updates);NativeSensorKeeper sensors(1);NativeSensorLifecycleHost actor(sensors);bool rejected=false;try{actor.initNerve(nullptr);}catch(const std::exception&){rejected=true;}assetCheck(rejected&&actor.mSpine==nullptr,"Null lifecycle nerve allocated state");actor.initNerve(&nerve);auto initialized=actor.mSpine;rejected=false;try{actor.initNerve(&nerve);}catch(const std::exception&){rejected=true;}assetCheck(rejected&&actor.mSpine==initialized,"Repeated lifecycle nerve initialization replaced owned state");actor.tickNerve();assetCheck(updates==1&&actor.getNerveStep()==1,"Live lifecycle host nerve did not advance");actor.mVelocity={1,2,3};actor.kill();actor.tickNerve();assetCheck(updates==1&&actor.getNerveStep()==1&&actor.mVelocity.x==0&&actor.mVelocity.y==0&&actor.mVelocity.z==0,"Dead host advanced nerve or retained velocity");actor.appear();actor.tickNerve();assetCheck(updates==2&&actor.getNerveStep()==2,"Appearance failed to resume retained nerve");}








