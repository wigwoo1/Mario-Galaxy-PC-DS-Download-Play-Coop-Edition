#include "Game/System/PauseButtonCheckerInGame.hpp"
#include <cassert>
#include <cstdio>
namespace { bool plus=false,minus=false,operating=false; }
namespace MR {
bool testCorePadButtonPlus(s32 c){assert(c==0);return plus;}
bool testCorePadButtonMinus(s32 c){assert(c==0);return minus;}
bool isOperatingWPad(s32 c){assert(c==0);return operating;}
}
int main(){
 PauseButtonCheckerInGame checker;
 assert(!checker.isPermitToPlusPause()&&!checker.isPermitToMinusPause());
 plus=true;
 for(int i=1;i<=13;++i){checker.update();assert(checker.isPermitToPlusPause()==(i==12));assert(!checker.isPermitToMinusPause());}
 plus=false;checker.update();plus=true;minus=true;
 for(int i=1;i<=11;++i){checker.update();assert(!checker.isPermitToPlusPause()&&!checker.isPermitToMinusPause());}
 operating=true;checker.update();assert(!checker.isPermitToPause()&&!checker.isPermitToPlusPause()&&!checker.isPermitToMinusPause());
 operating=false;assert(checker.isPermitToPlusPause()&&checker.isPermitToMinusPause());
 checker.update();assert(!checker.isPermitToPlusPause()&&!checker.isPermitToMinusPause());
 plus=false;minus=false;checker.update();minus=true;
 for(int i=1;i<=12;++i)checker.update();
 assert(!checker.isPermitToPlusPause()&&checker.isPermitToMinusPause());
 std::puts("PASS original pause checker: 12-frame threshold, independent buttons, release/reset and operation gating");
}
