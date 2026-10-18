#pragma once
#include "assets.hpp"
#include "Game/System/GameDataTemporaryInGalaxy.hpp"
inline void stageResultTests(){
    GameDataTemporaryInGalaxy result;assetCheck(!result.isStageCleared()&&result.mPowerStarId==1&&result.mStageName[0]==0&&result.mCoinNum==0,"Original stage result reset mismatch");
    result.receiveStageResultParam("EggStarGalaxy",3,12,321);assetCheck(result.isStageCleared()&&result.mPowerStarId==3&&std::string(result.mStageName)=="EggStarGalaxy"&&result.getStarPieceNum()==12&&result.mCoinNum==321,"Original stage result receive mismatch");
    result.addStarPiece(2000);assetCheck(result.getStarPieceNum()==999,"Original temporary Star Bit clamp mismatch");result.setLast1upStarPieceNum(50);assetCheck(result.mLast1upStarPieceNum==50,"Original temporary reward boundary mismatch");
    result.resetStageResultParam();assetCheck(!result.isStageCleared()&&result.getStarPieceNum()==0&&result.mCoinNum==0&&result.mLast1upStarPieceNum==0,"Original stage result reset retained old rewards");
    std::string longName(80,'x');result.receiveStageResultParam(longName.c_str(),2,1,1);assetCheck(std::strlen(result.mStageName)==47,"Original stage result name truncation mismatch");
}
