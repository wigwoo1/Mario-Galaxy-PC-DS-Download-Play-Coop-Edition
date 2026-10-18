#include "assets.hpp"
#include "Game/System/GameDataPlayerStatus.hpp"
namespace MR{int clamp(int,int,int);}
// Original status methods; native serialization below explicitly uses Wii byte order.
GameDataPlayerStatus::GameDataPlayerStatus() {
    initializeData();
}

u32 GameDataPlayerStatus::makeHeaderHashCode() const {
    return 0x27C90F;
}

u32 GameDataPlayerStatus::getSignature() const {
    return 'PLAY';
}

void GameDataPlayerStatus::initializeData() {
    mPlayerLeft = 4;
    mPlayerLeftSupply = 4;
    mStockedStarPiece = 0;
    mStoryProgress = 0;
}

s32 GameDataPlayerStatus::getPlayerLeft() const {
    s32 playerLeft = mPlayerLeft;

    return MR::clamp(playerLeft, 0, 99);
}

void GameDataPlayerStatus::addPlayerLeft(int num) {
    s32 playerLeft = mPlayerLeft + num;

    mPlayerLeft = MR::clamp(playerLeft, 0, 99);
}

bool GameDataPlayerStatus::isPlayerLeftSupply() const {
    return mPlayerLeftSupply >= 10;
}

void GameDataPlayerStatus::offPlayerLeftSupply() {
    mPlayerLeftSupply = 0;
}

void GameDataPlayerStatus::addStockedStarPiece(int num) {
    s32 stockedStarPiece = mStockedStarPiece + num;

    mStockedStarPiece = MR::clamp(stockedStarPiece, 0, 9999);
}

s32 GameDataPlayerStatus::serialize(u8* buffer,u32 size)const{assetCheck(buffer&&size>=7,"Player status output too small");buffer[0]=mStoryProgress;buffer[1]=u8(mStockedStarPiece>>24);buffer[2]=u8(mStockedStarPiece>>16);buffer[3]=u8(mStockedStarPiece>>8);buffer[4]=u8(mStockedStarPiece);buffer[5]=u8(mPlayerLeft>>8);buffer[6]=u8(mPlayerLeft);return 7;}
s32 GameDataPlayerStatus::deserialize(const u8* buffer,u32 size){assetCheck(buffer&&(size==5||size>=7),"Player status input truncated");initializeData();mStoryProgress=buffer[0];mStockedStarPiece=(u32(buffer[1])<<24)|(u32(buffer[2])<<16)|(u32(buffer[3])<<8)|buffer[4];if(size>=7)mPlayerLeftSupply=u16((u16(buffer[5])<<8)|buffer[6]);mPlayerLeft=4;return 0;}
