#pragma once
#include "assets.hpp"
#include <map>
#include "Game/Map/StageSwitch.hpp"
// Native storage adapter using original StageSwitch.cpp bit methods.
class NativeStageSwitchState {
    using Bits=BitFlag128;
    Bits global{};std::map<std::string,Bits> zones;
    static unsigned index(int32_t id){assetCheck(id>=0,"Missing stage switch");unsigned value=unsigned(id>=1000?id-1000:id);assetCheck(value<128,"Stage switch outside 128-bit bank");return value;}
public:
    void registerSwitch(const std::string& zone,int32_t id){index(id);if(id<1000)zones.try_emplace(zone,Bits{});}
    bool get(const std::string& zone,int32_t id)const{auto bit=index(id);const Bits* bank=&global;if(id<1000){auto found=zones.find(zone);if(found==zones.end())return false;bank=&found->second;}return bank->get(int(bit));}
    void set(const std::string& zone,int32_t id,bool enabled){auto bit=index(id);auto& bank=id>=1000?global:zones[zone];bank.set(int(bit),enabled);}
};
inline void stageSwitchStateTests(){NativeStageSwitchState state;for(int bit=0;bit<128;++bit){assetCheck(!state.get("a",bit)&&!state.get("a",1000+bit),"Switch bank was not zero initialized");state.set("a",bit,true);assetCheck(state.get("a",bit)&&!state.get("b",bit),"Local switch leaked across zones");state.set("a",1000+bit,true);assetCheck(state.get("b",1000+bit),"Global switch did not cross zones");state.set("a",bit,false);state.set("b",1000+bit,false);assetCheck(!state.get("a",bit)&&!state.get("a",1000+bit),"Switch clear failed");}for(int bad:{-1,128,999,1128}){bool rejected=false;try{state.set("a",bad,true);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Invalid switch index accepted");}}

#include <functional>
#include "native_scene.hpp"
#include "Game/Map/SwitchWatcher.hpp"
#include "Game/Map/ActorAppearSwitchListener.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Util/JMapInfo.hpp"
void bindNativeStageSwitchCtrl(const StageSwitchCtrl*,std::array<std::function<bool()>,3>,std::array<bool,4>);
void unbindNativeStageSwitchCtrl(const StageSwitchCtrl*);
void bindNativeStageSwitchRange(const StageSwitchCtrl*,std::function<bool(int,bool)>);
void bindNativeStageSwitchWrites(const StageSwitchCtrl*,std::array<std::function<void(bool)>,3>);
class NativeStageSwitchWatcher {
    NativeStageSwitchState& state;std::string zone;std::array<int32_t,3> ids;StageSwitchCtrl control{JMapInfoIter{}};NativeScene owner;SwitchWatcher* original=nullptr;bool ticking=false;
public:
    std::array<std::function<void(bool)>,3> listeners;
    struct Listener:SwitchEventListener{std::function<void(bool)>& callback;explicit Listener(std::function<void(bool)>& value):callback(value){}void listenSwitchOnEvent()override{callback(true);}void listenSwitchOffEvent()override{callback(false);}};
    std::array<std::unique_ptr<Listener>,3> adapters;
    NativeStageSwitchWatcher(NativeStageSwitchState& storage,std::string zoneName,std::array<int32_t,3> switches,int32_t deadID=-1):state(storage),zone(std::move(zoneName)),ids(switches){for(auto id:ids)if(id>=0)(void)state.get(zone,id);if(deadID>=0)(void)state.get(zone,deadID);for(auto id:ids)if(id>=0)state.registerSwitch(zone,id);if(deadID>=0)state.registerSwitch(zone,deadID);original=&owner.create<SwitchWatcher>(&control);for(size_t i=0;i<3;++i)adapters[i]=std::make_unique<Listener>(listeners[i]);std::array<std::function<bool()>,3> readers;for(size_t i=0;i<3;++i)readers[i]=[this,i](){return state.get(zone,ids[i]);};bindNativeStageSwitchCtrl(&control,std::move(readers),{ids[0]>=0,ids[1]>=0,ids[2]>=0,deadID>=0});std::array<std::function<void(bool)>,3> writers;for(size_t i=0;i<2;++i)writers[i]=[this,i](bool enabled){state.set(zone,ids[i],enabled);};writers[2]=[this,deadID](bool enabled){state.set(zone,deadID,enabled);};try{bindNativeStageSwitchWrites(&control,std::move(writers));bindNativeStageSwitchRange(&control,[this](int count,bool all){assetCheck(count>=0,"Negative switch range");if(count==0)return all;auto base=ids[1];assetCheck(base>=0,"Missing range switch B");int bit=base>=1000?base-1000:base;assetCheck(count<=128-bit,"Switch range exceeds bank");for(int i=0;i<count;++i)if(state.get(zone,base+i)!=all)return !all;return all;});}catch(...){unbindNativeStageSwitchCtrl(&control);throw;}}
    ~NativeStageSwitchWatcher(){unbindNativeStageSwitchCtrl(&control);}
    StageSwitchCtrl& switchControl(){return control;}
    void tick(){assetCheck(!ticking,"Recursive stage switch watcher update");ticking=true;try{for(size_t i=0;i<3;++i)original->addSwitchListener(listeners[i]&&ids[i]>=0?adapters[i].get():nullptr,1U<<i);original->movement();ticking=false;}catch(...){ticking=false;throw;}}
};
inline void stageSwitchWatcherTests(){NativeStageSwitchState state;for(size_t channel=0;channel<4;++channel){std::array<int32_t,3> invalid{1,2,3};int32_t dead=4;if(channel<3)invalid[channel]=128;else dead=1128;bool rejected=false;try{NativeStageSwitchWatcher malformed(state,"invalid",invalid,dead);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Malformed watcher channel accepted");assetCheck(!state.get("invalid",1)&&!state.get("invalid",2)&&!state.get("invalid",3),"Rejected watcher changed switch state");}NativeStageSwitchWatcher watcher(state,"a",{1,2,1000},4);assetCheck(watcher.switchControl().isValidSwitchDead(),"Authored dead switch validity lost");watcher.switchControl().onSwitchDead();assetCheck(state.get("a",4)&&!state.get("b",4),"Dead switch write lost zone isolation");watcher.switchControl().offSwitchDead();assetCheck(!state.get("a",4),"Dead switch clear failed");assetCheck(watcher.switchControl().isValidSwitchA()&&watcher.switchControl().isValidSwitchB()&&watcher.switchControl().isValidSwitchAppear(),"Authored switch validity lost");{NativeStageSwitchWatcher missing(state,"a",{-1,-1,-1});assetCheck(!missing.switchControl().isValidSwitchA()&&!missing.switchControl().isValidSwitchB()&&!missing.switchControl().isValidSwitchAppear()&&!missing.switchControl().isValidSwitchDead(),"Missing switch reported valid");bool rejected=false;try{missing.switchControl().onSwitchA();}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Missing switch write was accepted");}std::vector<int> events;for(size_t i=0;i<3;++i)watcher.listeners[i]=[&,i](bool enabled){events.push_back(int(i+1)*(enabled?1:-1));};watcher.tick();assetCheck(events.empty(),"Initially off switches emitted events");watcher.switchControl().onSwitchA();watcher.switchControl().onSwitchB();state.set("b",1000,true);watcher.tick();watcher.tick();assetCheck(events==std::vector<int>({1,2,3}),"Switch on ordering or unchanged suppression failed");watcher.switchControl().offSwitchA();watcher.switchControl().offSwitchB();state.set("a",1000,false);watcher.tick();assetCheck(events==std::vector<int>({1,2,3,-1,-2,-3}),"Switch off ordering failed");events.clear();watcher.listeners[0]=[&](bool enabled){events.push_back(enabled?1:-1);state.set("a",2,enabled);};state.set("a",1,true);watcher.tick();assetCheck(events==std::vector<int>({1,2}),"Callback switch change was not observed in channel order");bool first=true;watcher.listeners[2]=[&](bool){if(first){first=false;throw std::runtime_error("expected switch callback exception");}events.push_back(3);};state.set("a",1000,true);bool rejected=false;try{watcher.tick();}catch(const std::runtime_error&){rejected=true;}watcher.tick();assetCheck(rejected&&events.back()==3,"Failed switch callback did not retry");NativeStageSwitchWatcher recursive(state,"a",{5,-1,-1});int resumed=0;recursive.listeners[0]=[&](bool){recursive.tick();};state.set("a",5,true);rejected=false;try{recursive.tick();}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Recursive watcher update accepted");recursive.listeners[0]=[&](bool enabled){resumed+=enabled?1:-1;};recursive.tick();recursive.tick();assetCheck(resumed==1,"Watcher did not recover after recursive rejection");state.set("a",5,false);recursive.tick();assetCheck(resumed==0,"Recovered watcher lost off event");for(int base:{126,1126}){NativeStageSwitchWatcher range(state,"range",{-1,base,-1});auto& ctrl=range.switchControl();assetCheck(ctrl.isOnAllSwitchAfterB(0)&&!ctrl.isOnAnyOneSwitchAfterB(0),"Empty range mismatch");assetCheck(!ctrl.isOnAnyOneSwitchAfterB(2),"Off range mismatch");state.set("range",base+1,true);assetCheck(!ctrl.isOnAllSwitchAfterB(2)&&ctrl.isOnAnyOneSwitchAfterB(2),"Partial range mismatch");ctrl.onSwitchB();assetCheck(ctrl.isOnAllSwitchAfterB(2),"Full range mismatch");for(int count:{-1,3}){bool bad=false;try{ctrl.isOnAnyOneSwitchAfterB(count);}catch(const std::exception&){bad=true;}assetCheck(bad,"Invalid switch range accepted");}state.set("range",base,false);state.set("range",base+1,false);}}





#include "Game/LiveActor/LiveActor.hpp"
inline void originalAppearanceListenerTests(){struct Host:LiveActor{int appearances=0,kills=0;void appear()override{LiveActor::appear();++appearances;}void kill()override{LiveActor::kill();++kills;}};for(bool usesOn:{false,true})for(bool usesOff:{false,true}){Host actor;actor.kill();actor.kills=0;ActorAppearSwitchListener listener(&actor,usesOn,usesOff);listener.listenSwitchOnEvent();listener.listenSwitchOnEvent();assetCheck(actor.appearances==(usesOn?1:0)&&actor.isDead()==!usesOn,"Original appearance on policy mismatch");listener.listenSwitchOffEvent();listener.listenSwitchOffEvent();assetCheck(actor.kills==((usesOn&&usesOff)?1:0),"Original appearance off policy mismatch");}NativeStageSwitchState state;NativeStageSwitchWatcher watcher(state,"a",{-1,-1,3});Host actor;actor.kill();actor.kills=0;ActorAppearSwitchListener listener(&actor,true,true);watcher.listeners[2]=[&](bool enabled){listener.refresh(enabled);};state.set("a",3,true);watcher.tick();watcher.tick();assetCheck(!actor.isDead()&&actor.appearances==1,"Original watcher did not deliver appearance");state.set("a",3,false);watcher.tick();assetCheck(actor.isDead()&&actor.kills==1,"Original watcher did not deliver disappearance");}


#include "Game/Util/SwitchEventFunctorListener.hpp"
// Owns only NativeCallbackFunctor clones created through this wrapper.
class NativeSwitchFunctorListener:public SwitchEventListener {
    SwitchEventFunctorListener original;bool dispatching=false;
    void dispatch(bool enabled){assetCheck(!dispatching,"Recursive switch functor dispatch");dispatching=true;try{if(enabled)original.listenSwitchOnEvent();else original.listenSwitchOffEvent();dispatching=false;}catch(...){dispatching=false;throw;}}
    static void release(MR::FunctorBase* value){delete static_cast<NativeCallbackFunctor*>(value);}
public:
    NativeSwitchFunctorListener()=default;NativeSwitchFunctorListener(const NativeSwitchFunctorListener&)=delete;NativeSwitchFunctorListener& operator=(const NativeSwitchFunctorListener&)=delete;
    ~NativeSwitchFunctorListener(){release(original.mOnFunctor);release(original.mOffFunctor);}
    void on(std::function<void()> callback){assetCheck(!dispatching,"Switch functor replacement during callback");if(!callback){release(original.mOnFunctor);original.mOnFunctor=nullptr;return;}NativeCallbackFunctor functor(std::move(callback));auto previous=original.mOnFunctor;original.setOnFunctor(functor);release(previous);}
    void off(std::function<void()> callback){assetCheck(!dispatching,"Switch functor replacement during callback");if(!callback){release(original.mOffFunctor);original.mOffFunctor=nullptr;return;}NativeCallbackFunctor functor(std::move(callback));auto previous=original.mOffFunctor;original.setOffFunctor(functor);release(previous);}
    void listenSwitchOnEvent()override{dispatch(true);}
    void listenSwitchOffEvent()override{dispatch(false);}
};
inline void originalSwitchFunctorTests(){int on=0,off=0;NativeStageSwitchState state;NativeStageSwitchWatcher watcher(state,"a",{1,-1,-1});std::weak_ptr<int> lifetime;{NativeSwitchFunctorListener listener;listener.refresh(true);listener.refresh(false);auto owned=std::make_shared<int>(1);lifetime=owned;listener.on([&,owned](){on+=*owned;});owned.reset();listener.off([&](){++off;});watcher.listeners[0]=[&](bool value){listener.refresh(value);};watcher.switchControl().onSwitchA();watcher.tick();watcher.tick();watcher.switchControl().offSwitchA();watcher.tick();assetCheck(on==1&&off==1&&!lifetime.expired(),"Original cloned switch functor dispatch failed");listener.on([&](){on+=2;});assetCheck(lifetime.expired(),"Replaced switch functor clone retained captures");listener.on({});listener.refresh(true);assetCheck(on==1,"Cleared switch callback still executed");listener.on([&](){listener.on({});});bool rejected=false;try{listener.refresh(true);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Executing switch callback replaced itself");listener.on([&](){++on;});listener.refresh(true);assetCheck(on==2,"Callback replacement rejection did not recover");auto finalCapture=std::make_shared<int>(3);lifetime=finalCapture;listener.off([finalCapture](){});finalCapture.reset();watcher.listeners[0]={};}assetCheck(lifetime.expired(),"Switch functor owner leaked final capture");}





