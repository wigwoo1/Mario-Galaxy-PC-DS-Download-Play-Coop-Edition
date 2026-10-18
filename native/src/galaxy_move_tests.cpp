#include <Game/System/GalaxyMoveArgument.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
namespace MR { const JMapIdInfo& getInitializeStartIdInfo(){static JMapIdInfo i; i._0=17;i.mZoneID=3;return i;} bool isEqualString(const char*a,const char*b){return std::strcmp(a,b)==0;} }
int main(){JMapIdInfo id;id._0=42;id.mZoneID=9;GalaxyMoveArgument move(5,"TestGalaxy",2,&id);id._0=0;assert(move.mMoveType==5&&move.mScenarioNo==2&&move._C==-1&&move.mIDInfo._0==42&&move.mIDInfo.mZoneID==9);assert(move.isEqualStageScenario("TestGalaxy",2)&&!move.isEqualStageScenario("TestGalaxy",3)&&!move.isEqualStage("Other"));GalaxyMoveArgument initial(7,nullptr,1,nullptr);assert(initial.mIDInfo._0==17&&initial.mIDInfo.mZoneID==3&&!initial.isEqualStage("TestGalaxy"));std::puts("PASS original galaxy move: explicit ID copy, default start ID, stage/scenario matching and null stage");}
