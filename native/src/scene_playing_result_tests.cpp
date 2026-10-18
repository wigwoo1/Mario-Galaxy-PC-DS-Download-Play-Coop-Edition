#include <Game/Scene/ScenePlayingResult.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/System/GameDataFunction.hpp>
#include <cassert>
#include <cstdio>
// Observable boundary fixtures only; compiled production reward methods and
// NameObj registration are the original Petari source, not copied logic.
namespace {int lives=0,oneUps=0,purpleStars=0,bits=0,lastReward=0;bool astro=false;}
namespace MR {
 void notifyRequestNameObjMovementOnOff(){}
 void incPlayerLeft(){++lives;}
 void requestOneUp(){++oneUps;}
 void requestAppearPowerStarCoin100(){++purpleStars;}
 bool isStageAstroLocation(){return astro;}
}
namespace GameDataFunction {
 s32 getStarPieceNum(){return bits;}
 void addStarPiece(int count){bits+=count;}
 s32 getLast1upStarPieceNum(){return lastReward;}
 void setLast1upStarPieceNum(int count){lastReward=count;}
}
int main() {
 SingletonHolder<NameObjRegister>::init();
 NameObjHolder holder(4);
 auto registry=SingletonHolder<NameObjRegister>::get();registry->setCurrentHolder(&holder);
 ScenePlayingResult result;
 assert(holder.find(result.getName())==&result);
 assert(result.getCoinNum()==0&&result.mPurpleCoinNum==0);
 result.incCoin(49);assert(result.getCoinNum()==49&&lives==0);
 result.incCoin(1);assert(result.getCoinNum()==50&&lives==1&&oneUps==1);
 result.incCoin(1);assert(lives==1);
 result.incCoin(49);assert(result.getCoinNum()==100&&lives==2&&oneUps==2);
 result.incCoin(-200);assert(result.getCoinNum()==0&&lives==2);
 result.incCoin(2000);assert(result.getCoinNum()==999&&lives==3);
 result.incCoin(1);assert(result.getCoinNum()==999&&lives==3);
 for(int i=0;i<99;++i)result.incPurpleCoin();
 assert(result.mPurpleCoinNum==99&&purpleStars==0);
 result.incPurpleCoin();assert(purpleStars==1);
 result.incPurpleCoin();assert(purpleStars==1);
 lives=oneUps=0;
 result.incStarPiece(49);assert(bits==49&&lastReward==0&&lives==0);
 result.incStarPiece(1);assert(bits==50&&lastReward==50&&lives==1&&oneUps==1);
 result.incStarPiece(50);assert(bits==100&&lastReward==100&&lives==2);
 assert(result.getStarPieceNum()==100);
 // Reacquiring lost bits must not repeatedly award the same milestone.
 result.incStarPiece(-50);result.incStarPiece(50);
 assert(bits==100&&lastReward==100&&lives==2);
 astro=true;result.incStarPiece(50);assert(bits==150&&lastReward==100&&lives==2);
 astro=false;result.incStarPiece(1);assert(bits==151&&lastReward==150&&lives==2);
 result.incStarPiece(49);assert(bits==200&&lastReward==200&&lives==3);
 registry->setCurrentHolder(nullptr);
 std::puts("PASS original ScenePlayingResult: NameObj registration, coin bounds/50-coin 1-Up, 100-purple-coin trigger, Star Bit milestones/reacquisition/observatory suppression");
}

