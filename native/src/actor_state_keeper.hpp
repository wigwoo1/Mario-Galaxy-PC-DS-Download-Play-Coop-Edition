#pragma once
#include "assets.hpp"
#include "Game/LiveActor/ActorStateKeeper.hpp"
#include "Game/LiveActor/Spine.hpp"
class NativeActorStateKeeper {
    static int checkedCapacity(int capacity){assetCheck(capacity>0,"Invalid actor state capacity");return capacity;}
    ActorStateKeeper keeper;Spine* attached=nullptr;bool initializing=false;
public:
    explicit NativeActorStateKeeper(int capacity):keeper(checkedCapacity(capacity)){}
    NativeActorStateKeeper(const NativeActorStateKeeper&)=delete;
    NativeActorStateKeeper& operator=(const NativeActorStateKeeper&)=delete;
    ~NativeActorStateKeeper(){if(attached)attached->mStateKeeper=nullptr;delete[] keeper.mStates;}
    void attachTo(Spine& spine){assetCheck(!attached&&!spine.mStateKeeper,"Actor state keeper already attached");spine.mStateKeeper=&keeper;attached=&spine;}
    // State interfaces, nerves and names are borrowed for the keeper lifetime.
private:
    void validateRegistration(ActorStateBaseInterface* state,const Nerve* nerve,const char* name){assetCheck(!initializing,"Actor state registration during initialization");assetCheck(state&&nerve&&name,"Missing actor state registration");assetCheck(!keeper.findStateInfo(nerve),"Duplicate actor state nerve");assetCheck(keeper.mLength<keeper.mStatesCapacity,"Actor state capacity exceeded");}
public:
    void addState(ActorStateBaseInterface* state,const Nerve* nerve,const char* name){validateRegistration(state,nerve,name);keeper.addState(state,nerve,name);}
    void initState(ActorStateBaseInterface* state,const Nerve* nerve,const char* name){validateRegistration(state,nerve,name);initializing=true;try{state->init();initializing=false;}catch(...){initializing=false;throw;}keeper.addState(state,nerve,name);}
    void startState(const Nerve* nerve){keeper.startState(nerve);}
    void endState(const Nerve* nerve){keeper.endState(nerve);}
    bool updateCurrentState(){return keeper.updateCurrentState();}
};


