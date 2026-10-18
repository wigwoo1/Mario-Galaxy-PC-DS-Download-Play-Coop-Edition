#include "Game/Screen/WipeKoopa.hpp"
#include "Game/Screen/MissLayout.hpp"
#include "player_death_sequence.hpp"
namespace{NativePlayerEventServices* active=nullptr;NativePlayerEventServices& events(){assetCheck(active,"Missing player event services");return *active;}}
NativePlayerEventScope::NativePlayerEventScope(NativePlayerEventServices& services){assetCheck(!active&&services.stock,"Invalid player event service scope");active=&services;}
NativePlayerEventScope::~NativePlayerEventScope(){active=nullptr;}
namespace MR{
void startSoundPlayerJ(const char* name){events().sounds.emplace_back(name);}void startBckPlayerJ(const char* name){events().animations.emplace_back(name);}bool isMissLayoutAnimEnd(){return events().missLayout?!events().missLayout->isActive():events().missLayoutEnded;}void setPlayerSpot(f32 value,u32){assetCheck(std::isfinite(value),"Invalid player event spot value");}
void decPlayerLeft(){events().stock->addPlayerLeft(-1);++events().lifeDecrements;++events().missPointIncrements;++events().missCountIncrements;}
void deactivateDefaultGameLayout(){++events().layoutClosures;}void startDownWipe(){++events().downWipes;events().wipeActive=true;if(events().downWipe){events().downWipe->appear();events().downWipe->wipe(-1);}}bool isWipeActive(){return events().downWipe?(events().downWipe->isWipeIn()||events().downWipe->isWipeOut()):events().wipeActive;}void requestEndMissDemo(){++events().endMiss;}
void startPlayerDownWipe(){++events().playerDownWipes;}
void requestStartGameOverDemo(){++events().demoStarts;}void setCubeBgmChangeInvalid(){}void clearBgmQueue(){}void stopStageBGM(long){}void stopSubBGM(long){}void startMissLayout(){++events().missLayouts;events().missLayoutEnded=false;if(events().missLayout)events().missLayout->start();}void setSoundVolumeSetting(s32 setting,s32 frames){events().volumeSettings.emplace_back(setting,frames);}void startSubBGM(const char* name,bool){events().subBgm.emplace_back(name);}void startGameOverWipe(){++events().gameOverWipes;}void requestEndGameOverDemo(){++events().endGameOver;}
}
EventAbyss::EventAbyss() : EventSequence(12) {
    addEventOnTime("初期化", static_cast< EventFunc1 >(&EventAbyss::init), 0);
    addEventOnTime("通常レイアウト消去", static_cast< EventFunc1 >(&EventAbyss::closeDefaultLayout), 25);
    addEventOnTime("サウンドA", static_cast< EventFunc1 >(&EventAbyss::sound), 30);
    addEventInStatus("ワイプ開始", static_cast< EventFunc1 >(&EventAbyss::doCloseWipe), static_cast< EventFunc2 >(&EventAbyss::isMissLayoutClosed));
    addEventOnTime("残機を引く", static_cast< EventFunc1 >(&EventAbyss::decLeft), 120);
    addEventInPhase("ワイプ終了後", static_cast< EventFunc1 >(&EventAbyss::doWaitAfterWipe), 2);
}

void EventAbyss::init(u16 eventFrame, u16 sequenceFrame) {
    MR::requestStartGameOverDemo();
    playSound("声落下死亡");
    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();
    MR::stopStageBGM(10);
    MR::stopSubBGM(10);
    MR::startMissLayout();
}

void EventAbyss::sound(u16 eventFrame, u16 sequenceFrame) {
    MR::setSoundVolumeSetting(2, 20);
    MR::startSubBGM("BGM_MISS", false);
}

void EventAbyss::updateAfter() {
    setSpot(_28, _24);
}

EventGameOver::EventGameOver() : EventSequence(16) {
    addEventOnTime("初期化", static_cast< EventFunc1 >(&EventGameOver::init), 0);
    addEventOnTime("通常レイアウト消去", static_cast< EventFunc1 >(&EventGameOver::closeDefaultLayout), 60);
    addEventOnTime("サウンドA", static_cast< EventFunc1 >(&EventGameOver::sound), 90);
    addEventOnTime("レイアウト開始", static_cast< EventFunc1 >(&EventGameOver::reqLayout), 160);
    addEventOnTime("終了", static_cast< EventFunc1 >(&EventGameOver::finish), 420);
}

void EventGameOver::init(u16 eventFrame, u16 sequenceFrame) {
    MR::requestStartGameOverDemo();
    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();
    MR::stopStageBGM(10);
    MR::stopSubBGM(10);
    playSound("声最終ダメージ");
    playSound("最後の一撃");
    _28 = 20.0f;
}

void EventGameOver::sound(u16 eventFrame, u16 sequenceFrame) {
    switch (sequenceFrame) {
    case 90:
        MR::setSoundVolumeSetting(2, 30);
        MR::startSubBGM("BGM_GAMEOVER", false);
        break;
    }
}

void EventGameOver::updateAfter() {
}

void EventGameOver::reqLayout(u16 eventFrame, u16 sequenceFrame) {
    MR::startGameOverWipe();
}

void EventGameOver::finish(u16 eventFrame, u16 sequenceFrame) {
    MR::requestEndGameOverDemo();
    stopSequence();
}


// Original EventDown bodies; only MarioActor animation call is an explicit native request adapter.
namespace {void nativeRequestGameOverAnimation(){++events().gameOverAnimationRequests;}}
EventDown::EventDown() : EventSequence(16) {
    addEventOnTime("初期化", static_cast< EventFunc1 >(&EventDown::init), 0);
    addEventOnTime("MISSレイアウト開始", static_cast< EventFunc1 >(&EventDown::missLayoutOpen), 45);
    addEventOnTime("通常レイアウト消去", static_cast< EventFunc1 >(&EventDown::closeDefaultLayout), 30);
    addEventOnTime("サウンドA", static_cast< EventFunc1 >(&EventDown::sound), 30);
    addEventInTime("ワイプ開始", static_cast< EventFunc1 >(&EventDown::doCloseWipe), 100, 600);
    addEventOnTime("残機を引く", static_cast< EventFunc1 >(&EventDown::decLeft), 120);
    addEventInPhase("ワイプ終了後", static_cast< EventFunc1 >(&EventSequence::doWaitAfterWipe), 2);
}

void EventDown::init(u16 eventFrame, u16 sequenceFrame) {
    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();
    MR::stopStageBGM(10);
    MR::stopSubBGM(10);
    nativeRequestGameOverAnimation();
    playSound("声最終ダメージ");
    playSound("最後の一撃");
    MR::startPlayerDownWipe();
}
void EventDown::sound(u16 eventFrame, u16 sequenceFrame) {
    switch (sequenceFrame) {
    case 30:
        MR::setSoundVolumeSetting(2, 20);
        MR::startSubBGM("BGM_MISS", false);
        break;
    }
}
void EventDown::missLayoutOpen(u16 eventFrame, u16 sequenceFrame) {
    MR::startMissLayout();
    nextPhase();
}


// Original event bodies; FireDown MarioActor animation uses the explicit native request adapter.
EventFireDown::EventFireDown() : EventSequence(16) {
    addEventOnTime("初期化", static_cast< EventFunc1 >(&EventFireDown::init), 0);
    addEventOnTime("通常レイアウト消去", static_cast< EventFunc1 >(&EventFireDown::closeDefaultLayout), 100);
    addEventOnTime("サウンドA", static_cast< EventFunc1 >(&EventFireDown::sound), 30);
    addEventOnTime("サウンドB", static_cast< EventFunc1 >(&EventFireDown::sound2), 50);
    addEventInStatus("ワイプ開始", static_cast< EventFunc1 >(&EventFireDown::doCloseWipe),
                     static_cast< EventFunc2 >(&EventSequence::isMissLayoutClosed));
    addEventOnTime("残機を引く", static_cast< EventFunc1 >(&EventFireDown::decLeft), 120);
    addEventInPhase("ワイプ終了後", static_cast< EventFunc1 >(&EventFireDown::doWaitAfterWipe), 2);
}

void EventFireDown::init(u16 eventFrame, u16 sequenceFrame) {
    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();
    MR::stopStageBGM(10);
    MR::stopSubBGM(10);
    nativeRequestGameOverAnimation();
    playSound("声最終ダメージ");
    playSound("最後の一撃");
    _28 = 20.0f;
    MR::startPlayerDownWipe();
    MR::startMissLayout();
}

void EventFireDown::sound(u16 eventFrame, u16 sequenceFrame) {
    MR::setSoundVolumeSetting(2, 20);
    MR::startSubBGM("BGM_MISS", false);
}

void EventFireDown::updateAfter() {
    setSpot(_28, _24);
}

void EventFireDown::sound2(u16 eventFrame, u16 sequenceFrame) {
    MR::setSoundVolumeSetting(7, 70);
}

// Original event bodies; FireDown MarioActor animation uses the explicit native request adapter.
EventRaceDown::EventRaceDown() : EventSequence(16) {
    addEventOnTime("初期化", static_cast< EventFunc1 >(&EventRaceDown::init), 0);
    addEventOnTime("MISSレイアウト開始", static_cast< EventFunc1 >(&EventRaceDown::missLayoutOpen), 45);
    addEventOnTime("通常レイアウト消去", (&EventRaceDown::closeDefaultLayout), 30);
    addEventOnTime("サウンドA", static_cast< EventFunc1 >(&EventRaceDown::sound), 30);
    addEventOnTime("サウンドB", static_cast< EventFunc1 >(&EventRaceDown::sound), 100);
    addEventOnTime("サウンドC", static_cast< EventFunc1 >(&EventRaceDown::sound), 120);
    addEventInStatus("ワイプ開始", static_cast< EventFunc1 >(&EventRaceDown::doCloseWipe),
                     static_cast< EventFunc2 >(&EventRaceDown::checkCloseWipeStart));
    addEventOnTime("残機を引く", static_cast< EventFunc1 >(&EventRaceDown::decLeft), 120);
    addEventInPhase("ワイプ終了後", static_cast< EventFunc1 >(&EventRaceDown::doWaitAfterWipe), 2);
}

void EventRaceDown::init(u16, u16) {
    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();
    MR::stopStageBGM(10);
    MR::stopSubBGM(10);
    playAnimation("レース負け");
    playSound("声最終ダメージ");
    playSound("最後の一撃");
    MR::startPlayerDownWipe();
}

void EventRaceDown::sound(u16 eventFrame, u16 sequenceFrame) {
    switch (sequenceFrame) {
    case 30:
        MR::setSoundVolumeSetting(2, 20);
        MR::startSubBGM("BGM_MISS", false);
        break;
    case 100:
    case 120:
        break;
    }
}

bool EventRaceDown::checkCloseWipeStart(u16 sequenceFrame) {
    if (isMissLayoutClosed(sequenceFrame) && getPhase() == 1) {
        return true;
    } else {
        return false;
    }
}

void EventRaceDown::missLayoutOpen(u16 eventFrame, u16 sequenceFrame) {
    MR::startMissLayout();
    nextPhase();
}

// Original event bodies; FireDown MarioActor animation uses the explicit native request adapter.
EventGhostRaceDown::EventGhostRaceDown() : EventSequence(16) {
    addEventOnTime("初期化", static_cast< EventFunc1 >(&EventGhostRaceDown::init), 0);
    addEventOnTime("通常レイアウト消去", static_cast< EventFunc1 >(&EventGhostRaceDown::closeDefaultLayout), 30);
    addEventOnTime("サウンドA", static_cast< EventFunc1 >(&EventGhostRaceDown::sound), 30);
    addEventOnTime("サウンドB", static_cast< EventFunc1 >(&EventGhostRaceDown::sound), 120);
    addEventOnTime("サウンドC", static_cast< EventFunc1 >(&EventGhostRaceDown::sound), 140);
    addEventInTime("ワイプ開始", static_cast< EventFunc1 >(&EventGhostRaceDown::doCloseWipe), 92, 200);
    addEventOnTime("残機を引く", static_cast< EventFunc1 >(&EventGhostRaceDown::decLeft), 120);
    addEventInPhase("ワイプ終了後", static_cast< EventFunc1 >(&EventGhostRaceDown::doWaitAfterWipe), 2);
}

void EventGhostRaceDown::init(u16 eventFrame, u16 sequenceFrame) {
    MR::stopStageBGM(10);
    MR::stopSubBGM(10);
    MR::setCubeBgmChangeInvalid();
    playAnimation("レース負け");
    playSound("声最終ダメージ");
    playSound("最後の一撃");
}

void EventGhostRaceDown::sound(u16 eventFrame, u16 sequenceFrame) {
    switch (sequenceFrame) {
    case 30:
        MR::setSoundVolumeSetting(2, 20);
        MR::startSubBGM("BGM_MISS", false);
        break;
    case 100:
    case 120:
        break;
    }
}



