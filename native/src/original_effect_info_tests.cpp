#include "generated/AutoEffectInfo.cpp"
#include <cassert>
#include <cstdio>
int main(){AutoEffectInfo info;assert(!info.mGroupName&&!info.mUniqueName&&!info.mEffectName&&!info.mFlag);assert(info.mEndFrame==-1&&info.mScaleValue==1&&info.mRateValue==1&&!info.mIsValidPrmColor&&!info.mIsValidEnvColor);info.mEffectName="Smoke";assert(info.getName()==info.mEffectName);info.mUniqueName="SmokeLeft";assert(info.getName()==info.mUniqueName);
assert(str2Color("#123456")==0x12345600u);assert(str2Color("#FFFFFF")==0xffffff00u);assert(str2Color("#aBcDeF")==0xabcdef00u);assert(str2Color("#000000")==0);
const char* orders[]={"3D","PAUSE_IGNORE","INDIRECT","AFTER_INDIRECT","BLOOM_EFFECT","AFTER_IMAGE_EFFECT","2D","2D_PAUSE_IGNORE","FOR_2D_MODEL"};for(int i=0;i<9;++i)assert(getDrawOrderData(orders[i])==i);assert(getDrawOrderData("unknown")==0);puts("PASS original effect metadata defaults, name precedence, packed colors and nine draw orders");}
