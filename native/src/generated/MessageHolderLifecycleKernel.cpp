#include <Game/System/MessageHolder.hpp>
#include <Native/GameMessages.hpp>
MessageHolder::MessageHolder() : mSystemMessageData(nullptr), mGameMessageData(nullptr), mSceneMessageData(nullptr) {
}

void MessageHolder::initSceneData() {
    mSceneMessageData = mGameMessageData; nativeBindSceneMessages(*this);
}

void MessageHolder::destroySceneData() {
    nativeUnbindSceneMessages(*this); mSceneMessageData = nullptr;
}

void MessageHolder::initSystemData(){nativeInitGameMessages(*this,true);}
void MessageHolder::initGameData(){nativeInitGameMessages(*this,false);}

MessageData* MessageSystem::getSceneMessageData(){return nativeSceneMessages();}
