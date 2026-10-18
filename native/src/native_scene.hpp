#pragma once
#include "Game/NameObj/NameObj.hpp"
#include "Game/NameObj/NameObjAdaptor.hpp"
#include "Game/NameObj/NameObjRegister.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <memory>
#include <functional>
class NativeScene {
    NameObjHolder holder;std::vector<std::unique_ptr<NameObj>> owned;bool ticking=false;
public:
    template<class Actor,class... Args> Actor& create(Args&&... args){
        assetCheck(!ticking,"Actor creation during movement is not supported");SingletonHolder<NameObjRegister>::init();auto registry=SingletonHolder<NameObjRegister>::get();auto previous=registry->mHolder;size_t count=holder.objects.size();registry->setCurrentHolder(&holder);
        try{auto actor=std::make_unique<Actor>(std::forward<Args>(args)...);registry->setCurrentHolder(previous);auto& result=*actor;owned.push_back(std::move(actor));return result;}
        catch(...){registry->setCurrentHolder(previous);holder.objects.resize(count);throw;}
    }
    void remove(NameObj& actor){assetCheck(!ticking,"Actor removal during frame update");auto found=std::find_if(owned.begin(),owned.end(),[&](const auto& value){return value.get()==&actor;});assetCheck(found!=owned.end(),"Actor is not owned by this scene");holder.objects.erase(std::remove(holder.objects.begin(),holder.objects.end(),&actor),holder.objects.end());owned.erase(found);}
    void tick(){assetCheck(!ticking,"Recursive scene movement");ticking=true;try{for(auto actor:holder.objects)actor->syncWithFlags();for(auto actor:holder.objects)actor->executeMovement();ticking=false;}catch(...){ticking=false;throw;}}
    void renderPhases(){
        assetCheck(!ticking,"Recursive scene frame phase");ticking=true;
        try{for(auto actor:holder.objects)actor->calcAnim();for(auto actor:holder.objects)actor->calcViewAndEntry();for(auto actor:holder.objects)actor->draw();ticking=false;}catch(...){ticking=false;throw;}
    }
    template<class Actor,class... Args> Actor& place(const JMapInfoIter& iterator,Args&&... args){
        size_t count=holder.objects.size(),ownedCount=owned.size();auto& actor=create<Actor>(std::forward<Args>(args)...);ticking=true;
        try{actor.init(iterator);actor.initAfterPlacement();ticking=false;return actor;}
        catch(...){ticking=false;holder.objects.resize(count);owned.resize(ownedCount);throw;}
    }
    size_t size()const{return owned.size();}
    ~NativeScene(){owned.clear();holder.objects.clear();}
};
struct NativeCallbackFunctor:MR::FunctorBase {
    std::function<void()> update;
    explicit NativeCallbackFunctor(std::function<void()> callback):update(std::move(callback)){}
    void operator()()const override{update();}
    MR::FunctorBase* clone(JKRHeap* heap)const override{assetCheck(heap==nullptr,"Native callback does not support Wii heaps");return new NativeCallbackFunctor(*this);}
};
struct NativeCallbackActor:NameObjAdaptor {
    explicit NativeCallbackActor(std::function<void()> callback):NameObjAdaptor("native diagnostic actor"){NativeCallbackFunctor functor(std::move(callback));connectToMovement(functor);}
};
inline void nativeSceneTests(){
    struct PhaseActor:NameObj {
        std::vector<int>* order;int id;NativeScene* scene;bool recurse=false;
        PhaseActor(std::vector<int>* output,int number,NativeScene* runtime):NameObj("phase probe"),order(output),id(number),scene(runtime){}
        void movement()override{order->push_back(id);}
        void calcAnim()override{order->push_back(10+id);if(recurse)scene->renderPhases();}
        void calcViewAndEntry()override{order->push_back(20+id);}
        void draw()const override{order->push_back(30+id);}
    };
    std::vector<int> order;NativeScene phases;auto& phaseActor=phases.create<PhaseActor>(&order,1,&phases);phases.create<PhaseActor>(&order,2,&phases);
    phases.tick();phases.renderPhases();assetCheck(order==std::vector<int>({1,2,11,12,21,22,31,32}),"Scene frame phase ordering failed");
    order.clear();phaseActor.requestSuspend();phases.tick();phases.renderPhases();assetCheck(order==std::vector<int>({2,11,12,21,22,31,32}),"Movement suspension incorrectly suppressed rendering");
    phaseActor.recurse=true;bool rejected=false;try{phases.renderPhases();}catch(const std::runtime_error&){rejected=true;}phaseActor.recurse=false;order.clear();phases.renderPhases();assetCheck(rejected&&order==std::vector<int>({11,12,21,22,31,32}),"Frame recursion rejection/recovery failed");
    struct Lifetime {int* count;explicit Lifetime(int* value):count(value){}~Lifetime(){++*count;}};
    int destroyed=0,animation=0,draw=0;auto lifetime=std::make_shared<Lifetime>(&destroyed);
    {NativeScene callbacks;auto& adapter=callbacks.create<NativeCallbackActor>([lifetime](){});lifetime.reset();NativeCallbackFunctor anim([&](){++animation;}),drawing([&](){++draw;});adapter.connectToCalcAnim(anim);adapter.connectToDraw(drawing);adapter.calcAnim();adapter.draw();assetCheck(destroyed==0&&animation==1&&draw==1,"Original adaptor cloned callback dispatch failed");}
    assetCheck(destroyed==1,"Original adaptor callback ownership destruction failed");
    int updates=0;NativeScene first,second;auto& actor=first.create<NativeCallbackActor>([&](){++updates;});second.create<NativeCallbackActor>([&](){updates+=10;});
    assetCheck(first.size()==1&&second.size()==1,"Scene ownership/registration test");first.tick();assetCheck(updates==1,"Scene isolation test");actor.requestSuspend();first.tick();assetCheck(updates==1,"Scene suspended actor moved");actor.requestResume();first.tick();second.tick();assetCheck(updates==12,"Scene original actor resume test");first.remove(actor);first.tick();assetCheck(first.size()==0&&updates==12,"Removed scene actor kept updating");auto& foreign=second.create<NativeCallbackActor>([](){});bool foreignRejected=false;try{first.remove(foreign);}catch(const std::exception&){foreignRejected=true;}assetCheck(foreignRejected&&second.size()==2,"Foreign actor removal mutated scene ownership");
    bool thrown=false;NativeScene exceptional;exceptional.create<NativeCallbackActor>([&](){if(!thrown){thrown=true;throw std::runtime_error("expected actor exception");}});try{exceptional.tick();}catch(const std::runtime_error&){}exceptional.tick();assetCheck(thrown,"Scene exception recovery test");
}

