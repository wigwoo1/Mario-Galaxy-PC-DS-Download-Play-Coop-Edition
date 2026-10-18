#pragma once
#include "assets.hpp"
#include "player_stock.hpp"
#include "Game/Player/PlayerEventAbyss.hpp"
#include "Game/Player/PlayerEventGameOver.hpp"
#include "Game/Player/PlayerEventDown.hpp"
#include "Game/Player/PlayerEventFireDown.hpp"
#include "Game/Player/PlayerEventRaceDown.hpp"
#include "Game/Player/PlayerEventGhostRaceDown.hpp"

#include <memory>
class MissLayout;class WipeKoopa;
enum class NativePlayerDeathKind{Abyss,Normal,Fire,Race,GhostRace};
struct NativePlayerEventServices{
    NativePlayerStock* stock=nullptr;MissLayout* missLayout=nullptr;WipeKoopa* downWipe=nullptr;bool missLayoutEnded=false,wipeActive=false;
    int gameOverAnimationRequests=0,playerDownWipes=0,demoStarts=0,missLayouts=0,downWipes=0,gameOverWipes=0,endMiss=0,endGameOver=0,lifeDecrements=0,missPointIncrements=0,missCountIncrements=0,layoutClosures=0;
    std::vector<std::string> sounds,animations,subBgm;std::vector<std::pair<s32,s32>> volumeSettings;
};
class NativePlayerEventScope{public:explicit NativePlayerEventScope(NativePlayerEventServices&);~NativePlayerEventScope();NativePlayerEventScope(const NativePlayerEventScope&)=delete;NativePlayerEventScope& operator=(const NativePlayerEventScope&)=delete;};
// Diagnostic runtime driver for the original EventSequence bodies. Retail
// EventSequencer is a scene NameObj; this owned test/runtime driver avoids
// registering a stack/member object outside the retail scene lifecycle.
class NativePlayerEventSequencer{
    std::vector<std::pair<std::string,EventSequence*>> sequences;
    EventSequence* current=nullptr;u16 frame=0;
public:
    void registerSequence(const char* name,EventSequence& sequence){assetCheck(name&&*name&&!current,"Invalid player event registration");for(const auto& entry:sequences)assetCheck(entry.first!=name,"Duplicate player event name");sequences.emplace_back(name,&sequence);}
    void startEvent(const char* name){assetCheck(name,"Missing player event name");for(const auto& entry:sequences)if(entry.first==name){current=entry.second;frame=0;current->clearFlag();return;}}
    void movement(){if(current&&current->checkAndRun(frame))current=nullptr;++frame;}
    bool running()const{return current!=nullptr;}
    u16 nextFrame()const{return frame;}
};
class NativePlayerDeathSequence{
    EventAbyss abyss;EventGameOver gameOver;EventDown down;EventFireDown fire;EventRaceDown race;EventGhostRaceDown ghostRace;NativePlayerEventSequencer sequencer;MissLayout* layout=nullptr;WipeKoopa* wipe=nullptr;
public:
    NativePlayerEventServices services;
    NativePlayerDeathSequence(){abyss._24=0;abyss._28=20;gameOver._24=0;gameOver._28=20;sequencer.registerSequence("abyss",abyss);sequencer.registerSequence("game-over",gameOver);sequencer.registerSequence("down",down);fire._24=0;fire._28=20;sequencer.registerSequence("fire",fire);sequencer.registerSequence("race",race);sequencer.registerSequence("ghost-race",ghostRace);}
    bool start(NativePlayerStock& stock,NativePlayerDeathKind kind=NativePlayerDeathKind::Abyss){
        if(sequencer.running())return false;
        const char* name=nullptr;
        switch(kind){case NativePlayerDeathKind::Abyss:name="abyss";break;case NativePlayerDeathKind::Normal:name="down";break;case NativePlayerDeathKind::Fire:name="fire";break;case NativePlayerDeathKind::Race:name="race";break;case NativePlayerDeathKind::GhostRace:name="ghost-race";break;}
        assetCheck(name,"Invalid player death kind");services={};services.stock=&stock;services.missLayout=layout;services.downWipe=wipe;sequencer.startEvent(stock.getPlayerLeft()==0?"game-over":name);return true;
    }
    void bindMissLayout(MissLayout& actor){assetCheck(!running(),"Binding miss layout during player event");layout=&actor;}
    void bindDownWipe(WipeKoopa& actor){assetCheck(!running(),"Binding wipe during player event");wipe=&actor;}
    bool running()const{return sequencer.running();}
    uint32_t nextFrame()const{return sequencer.nextFrame();}
    void tick(){if(!sequencer.running())return;NativePlayerEventScope scope(services);sequencer.movement();}
};
inline void playerDeathSequenceTests(){
    NativePlayerStock stock;NativePlayerDeathSequence sequence;assetCheck(sequence.start(stock)&&!sequence.start(stock),"Duplicate player death sequence accepted");
    for(int i=0;i<120;++i)sequence.tick();assetCheck(stock.getPlayerLeft()==4&&sequence.services.demoStarts==1&&sequence.services.missLayouts==1&&sequence.services.layoutClosures==1&&sequence.services.downWipes==0,"Original abyss event timing/layout gate mismatch");
    sequence.tick();assetCheck(stock.getPlayerLeft()==3&&sequence.services.lifeDecrements==1&&sequence.services.missPointIncrements==1&&sequence.services.missCountIncrements==1,"Original abyss frame-120 life decrement mismatch");
    sequence.services.missLayoutEnded=true;sequence.tick();assetCheck(sequence.services.downWipes==1&&sequence.running(),"Original abyss wipe start mismatch");sequence.services.wipeActive=false;
    for(int i=0;i<31;++i)sequence.tick();assetCheck(!sequence.running()&&sequence.services.endMiss==1&&sequence.services.lifeDecrements==1,"Original abyss post-wipe phase/end mismatch");sequence.tick();assetCheck(stock.getPlayerLeft()==3,"Completed death sequence decremented again");
    assetCheck(sequence.start(stock),"Death sequence could not restart");for(int i=0;i<=120;++i)sequence.tick();assetCheck(stock.getPlayerLeft()==2&&sequence.services.lifeDecrements==1,"Restarted abyss sequence retained event frames");
    NativePlayerStock empty;empty.addPlayerLeft(-99);NativePlayerDeathSequence over;over.start(empty);for(int i=0;i<420;++i)over.tick();assetCheck(over.running()&&over.services.gameOverWipes==1&&over.services.endGameOver==0&&over.services.lifeDecrements==0,"Original game-over timing mismatch");over.tick();assetCheck(!over.running()&&over.services.endGameOver==1&&empty.getPlayerLeft()==0,"Original frame-420 game-over end mismatch");
    NativePlayerStock damageStock;NativePlayerDeathSequence damage;damage.start(damageStock,NativePlayerDeathKind::Normal);
    for(int i=0;i<30;++i)damage.tick();assetCheck(damage.services.gameOverAnimationRequests==1&&damage.services.playerDownWipes==1&&damage.services.missLayouts==0&&damage.services.layoutClosures==0,"Normal death initial event timing mismatch");
    damage.tick();assetCheck(damage.services.layoutClosures==1&&damage.services.subBgm.size()==1,"Normal death frame-30 sound/layout mismatch");
    for(int i=31;i<45;++i)damage.tick();assetCheck(damage.services.missLayouts==0,"Normal death miss layout opened early");damage.tick();assetCheck(damage.services.missLayouts==1,"Normal death frame-45 miss layout mismatch");
    for(int i=46;i<100;++i)damage.tick();assetCheck(damage.services.downWipes==0,"Normal death wipe started early");damage.tick();assetCheck(damage.services.downWipes==1,"Normal death frame-100 wipe mismatch");
    for(int i=101;i<=120;++i)damage.tick();assetCheck(damageStock.getPlayerLeft()==3&&damage.services.lifeDecrements==1,"Normal death frame-120 life decrement mismatch");
    damage.services.wipeActive=false;for(int i=0;i<31;++i)damage.tick();assetCheck(!damage.running()&&damage.services.endMiss==1,"Normal death gated completion mismatch");
    NativePlayerDeathSequence damageOver;damageOver.start(empty,NativePlayerDeathKind::Normal);damageOver.tick();assetCheck(damageOver.services.demoStarts==1&&damageOver.services.playerDownWipes==0,"Zero-life normal death did not select game over");
    for(auto kind:{NativePlayerDeathKind::Fire,NativePlayerDeathKind::Race,NativePlayerDeathKind::GhostRace}){
        NativePlayerStock eventStock;NativePlayerDeathSequence event;event.start(eventStock,kind);
        for(int i=0;i<=120;++i)event.tick();
        assetCheck(eventStock.getPlayerLeft()==3&&event.services.lifeDecrements==1,"Failure variant frame-120 life loss mismatch");
        assetCheck(event.services.layoutClosures==1&&event.services.subBgm.size()==1,"Failure variant layout/sound dispatch mismatch");
        if(kind==NativePlayerDeathKind::Fire){assetCheck(event.services.gameOverAnimationRequests==1&&event.services.missLayouts==1&&event.services.downWipes==0&&event.services.volumeSettings==std::vector<std::pair<s32,s32>>{{2,20},{7,70}},"Fire death animation/layout/wipe gate mismatch");}
        else{assetCheck(event.services.animations.size()==1&&event.services.gameOverAnimationRequests==0,"Race death animation dispatch mismatch");}
        if(kind==NativePlayerDeathKind::GhostRace){assetCheck(event.services.missLayouts==0&&event.services.playerDownWipes==0&&event.services.downWipes==1,"Ghost race wipe interval mismatch");}
        else{assetCheck(event.services.playerDownWipes==1&&event.services.downWipes==0,"Failure variant bypassed miss-layout gate");event.services.missLayoutEnded=true;event.tick();assetCheck(event.services.downWipes==1,"Failure variant closing wipe not requested");}
        event.services.wipeActive=false;for(int i=0;i<31;++i)event.tick();assetCheck(!event.running()&&event.services.endMiss==1,"Failure variant post-wipe completion mismatch");
        NativePlayerDeathSequence noLives;noLives.start(empty,kind);noLives.tick();assetCheck(noLives.services.demoStarts==1&&noLives.services.animations.empty()&&noLives.services.playerDownWipes==0,"Failure variant zero-stock game-over routing mismatch");
    }
    struct Interval:EventSequence{std::vector<std::pair<u16,u16>> calls;Interval():EventSequence(1){addEventInTime("test",static_cast<EventFunc1>(&Interval::record),1,3);}void record(u16 event,u16 sequence){calls.emplace_back(event,sequence);}} interval;for(int i=0;i<5;++i)interval.checkAndRun(i);assetCheck(interval.calls==std::vector<std::pair<u16,u16>>{{0,1},{1,2},{2,3}},"Original inclusive event interval mismatch");bool rejected=false;try{interval.addEventOnTime("overflow",static_cast<EventFunc1>(&Interval::record),0);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Player event registration overflow accepted");
}





