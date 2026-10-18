#pragma once
#include "assets.hpp"
#include "player_stock.hpp"
#include "native_scene.hpp"
#include "Game/Scene/ScenePlayingResult.hpp"
struct NativeCoinRewardEvents{int lifeIncrements=0,oneUpRequests=0,purpleStarRequests=0;std::function<void()> deliverLife;};
void bindNativeCoinRewardEvents(NativeCoinRewardEvents*);
void unbindNativeCoinRewardEvents();
class NativeCoinRewardScope{public:explicit NativeCoinRewardScope(NativeCoinRewardEvents& events){bindNativeCoinRewardEvents(&events);}~NativeCoinRewardScope(){unbindNativeCoinRewardEvents();}NativeCoinRewardScope(const NativeCoinRewardScope&)=delete;NativeCoinRewardScope& operator=(const NativeCoinRewardScope&)=delete;};
inline void coinRewardTests(){NativeScene scene;auto& result=scene.create<ScenePlayingResult>();NativeCoinRewardEvents events;NativePlayerStock stock;events.deliverLife=[&](){stock.addPlayerLeft(1);};NativeCoinRewardScope scope(events);result.incCoin(49);assetCheck(result.getCoinNum()==49&&events.lifeIncrements==0,"Coin reward below threshold mismatch");result.incCoin(1);assetCheck(events.lifeIncrements==1&&events.oneUpRequests==1&&stock.getPlayerLeft()==5,"Original fifty-coin reward mismatch");result.incCoin(100);assetCheck(events.lifeIncrements==2,"Original batch coin threshold policy changed");result.incCoin(999);assetCheck(result.getCoinNum()==999&&events.lifeIncrements==3&&stock.getPlayerLeft()==7,"Coin cap/reward mismatch");result.incCoin(-2000);assetCheck(result.getCoinNum()==0&&events.lifeIncrements==3,"Coin floor granted reward");for(int i=0;i<99;++i)result.incPurpleCoin();assetCheck(events.purpleStarRequests==0,"Purple coin star requested early");result.incPurpleCoin();result.incPurpleCoin();assetCheck(events.purpleStarRequests==1,"Purple coin star request count mismatch");}

