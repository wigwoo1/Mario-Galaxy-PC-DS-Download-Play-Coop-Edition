#pragma once
#include "Game/NameObj/NameObj.hpp"
#include "Game/LiveActor/Spine.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>
class J3DFrameCtrl;
// Native lifecycle/animation-request adapter, not original LayoutManager or Wii ABI.
class LayoutActor:public NameObj {
    std::unique_ptr<Spine> spine;std::unique_ptr<J3DFrameCtrl> animationClock;std::map<std::string,std::pair<u16,bool>> animationTiming;
public:
    bool nativeDead=true,nativeHidden=true,nativeAnimStopped=false;
    std::string nativeLayout,nativeAnimation;
    std::vector<std::string> nativeEffects;
    LayoutActor(const char*,bool);~LayoutActor() override;
    void registerNativeAnimation(const std::string&,u16,bool);void startNativeAnimation(const char*);float nativeAnimationFrame()const;J3DFrameCtrl* nativeAnimationController(){return animationClock.get();}
    void movement() override;
    virtual void appear();virtual void kill();
    void initNerve(const Nerve*);void setNerve(const Nerve*)const;bool isNerve(const Nerve*)const;s32 getNerveStep()const;
    void initLayoutManager(const char*,u32);void initEffectKeeper(int,const char*,const void*);
};


