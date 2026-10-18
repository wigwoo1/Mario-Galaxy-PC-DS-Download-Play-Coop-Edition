#include "player_death_sequence.hpp"
#include <memory>

namespace MR{
void startSoundPlayerJ(const char*);void startBckPlayerJ(const char*);bool isMissLayoutAnimEnd();void setPlayerSpot(f32,u32);void decPlayerLeft();void deactivateDefaultGameLayout();void startDownWipe();bool isWipeActive();void requestEndMissDemo();
}
// Native bounded, exception-safe allocation; original dispatch/timing bodies follow.
EventSequence::EventSequence(u32 count):mEventNumMax(u16(count)),mEventNum(0){assetCheck(count>0&&count<=256,"Invalid player event capacity");auto funcs=std::make_unique<EventFunc1[]>(count);auto checks=std::make_unique<EventFunc2[]>(count);auto starts=std::make_unique<u16[]>(count);auto stops=std::make_unique<u16[]>(count);auto frames=std::make_unique<u16[]>(count);mEventFuncs=funcs.release();mEventCheckFuncs=checks.release();mStartFrameOrModes=starts.release();mStopFrameOrPhases=stops.release();mEventFrames=frames.release();clearFlag();}
EventSequence::~EventSequence(){delete[] mEventFuncs;delete[] mEventCheckFuncs;delete[] mStartFrameOrModes;delete[] mStopFrameOrPhases;delete[] mEventFrames;}
void EventSequence::clearFlag() { 
    for (u32 i = 0; i < mEventNumMax; i++) {
        mEventFrames[i] = 0;
    }

    mPhase = 0;
    mFrameInPhase = 0;
    mNextPhase = false;
}

void EventSequence::addEventOnTime(const char* pName, EventFunc1 eventFn, u16 frame) {
    assetCheck(mEventNum<mEventNumMax&&eventFn!=nullptr,"Invalid/overflowing player event registration");
    mEventFuncs[mEventNum] = eventFn;
    mStartFrameOrModes[mEventNum] = frame;
    mStopFrameOrPhases[mEventNum] = frame;
    mEventNum++;
}

void EventSequence::addEventInTime(const char* pName, EventFunc1 eventFn, u16 startFrame, u16 stopFrame) {
    assetCheck(mEventNum<mEventNumMax&&eventFn!=nullptr,"Invalid/overflowing player event registration");
    mEventFuncs[mEventNum] = eventFn;
    mStartFrameOrModes[mEventNum] = startFrame;
    mStopFrameOrPhases[mEventNum] = stopFrame;
    mEventNum++;
}

void EventSequence::addEventInStatus(const char* pName, EventFunc1 eventFn, EventFunc2 eventCheckFn) {
    assetCheck(mEventNum<mEventNumMax&&eventFn!=nullptr,"Invalid/overflowing player event registration");
    mEventFuncs[mEventNum] = eventFn;
    mStartFrameOrModes[mEventNum] = 0xFFFF;
    mStopFrameOrPhases[mEventNum] = 0xFFFF;
    mEventCheckFuncs[mEventNum] = eventCheckFn;
    mEventNum++;
}

void EventSequence::addEventInPhase(const char* pName, EventFunc1 eventFn, u16 phase) {
    assetCheck(mEventNum<mEventNumMax&&eventFn!=nullptr,"Invalid/overflowing player event registration");
    mEventFuncs[mEventNum] = eventFn;
    mStartFrameOrModes[mEventNum] = 0xFFFE;
    mStopFrameOrPhases[mEventNum] = phase;
    mEventCheckFuncs[mEventNum] = nullptr;
    mEventNum++;
}

bool EventSequence::checkAndRun(u32 sequenceFrame) {
    mStopSequence = false;

    updateBefore();

    for (u32 i = 0; i < mEventNum; i++) {
        u16 minFrame = mStartFrameOrModes[i];
        if (minFrame == 0xFFFF) {  // EventInStatus
            if ((mStopFrameOrPhases[i] == 0 && mEventFrames[i] != 0)) {
                continue;
            }
            if (mEventCheckFuncs[i] && !(this->*mEventCheckFuncs[i])(sequenceFrame)) {
                continue;
            }
        } else if (minFrame == 0xFFFE) {  // EventInPhase
            if (mPhase != mStopFrameOrPhases[i]) {
                continue;
            }
        } else {  // EventOnTime or EventInTime
            if (minFrame > sequenceFrame || sequenceFrame > mStopFrameOrPhases[i]) {
                continue;
            }
        }

        (this->*mEventFuncs[i])(mEventFrames[i], sequenceFrame);
        mEventFrames[i]++;
        if (mStopSequence) {
            break;
        }
    }

    updateAfter();

    if (mNextPhase) {
        mPhase++;
        mFrameInPhase = 0;
        mNextPhase = false;
    } else {
        mFrameInPhase++;
    }

    return mStopSequence;
}

void EventSequence::playSound(const char* pName) {
    MR::startSoundPlayerJ(pName);
}

void EventSequence::playAnimation(const char* pName) {
    MR::startBckPlayerJ(pName);
}

bool EventSequence::isMissLayoutClosed(u16 sequenceFrame) {
    return (MR::isMissLayoutAnimEnd() && sequenceFrame > 15);
}

void EventSequence::setSpot(f32 param1, u32 param2) {
    MR::setPlayerSpot(param1, param2);
}

void EventSequence::decLeft(u16 eventFrame, u16 sequenceFrame) {
    MR::decPlayerLeft();
}

void EventSequence::closeDefaultLayout(u16 eventFrame, u16 sequenceFrame) {
    MR::deactivateDefaultGameLayout();
}

void EventSequence::doCloseWipe(u16 eventFrame, u16 sequenceFrame) {
    if (eventFrame == 0) {
        MR::startDownWipe();
    } else if (!MR::isWipeActive()) {
        mPhase = 2;
    }
}

void EventSequence::doWaitAfterWipe(u16 eventFrame, u16 sequenceFrame) {
    if (eventFrame == 30) {
        MR::requestEndMissDemo();
        stopSequence();
    }
}


void EventSequence::updateBefore(){};
void EventSequence::updateAfter(){};

void EventSequencer::registerSequence(const char* name,EventSequence& sequence){
    assetCheck(name&&*name&&!running(),"Invalid player event registration");
    for(const auto& entry:sequences)assetCheck(entry.first!=name,"Duplicate player event name");
    assetCheck(sequences.size()<16,"Player event registry capacity exceeded");
    sequences.emplace_back(name,&sequence);
}
void EventSequencer::startEvent(const char* name){
    assetCheck(name,"Missing player event name");
    for(const auto& entry:sequences)if(entry.first==name){mSequence=entry.second;mSequenceFrame=0;mSequence->clearFlag();return;}
}
// Unchanged original EventSequencer movement body.
void EventSequencer::movement() {
    if (mSequence == nullptr) {
        return;
    }
    if (mSequence->checkAndRun(mSequenceFrame)) {
        mSequence = nullptr;
    }
    mSequenceFrame++;
}
