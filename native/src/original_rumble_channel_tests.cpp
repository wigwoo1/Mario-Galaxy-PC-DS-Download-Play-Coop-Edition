#include "Game/System/WPadRumble.hpp"
#include "Game/System/WPadRumbleData.hpp"
#include <cassert>
#include <cstdio>
int main(){
 RumbleChannel c{};c.clear();c.update();assert(!c._0&&!c._E);
 RumblePattern p{};p.mFrame=4;p.mPattern[0]=1;p.mPattern[1]=0;p.mPattern[2]=1;p.mPattern[3]=1;
 int owner=0;c.setPattern(&owner,p,7,false);assert(c._8==7&&c._10==&owner);
 for(int i=0;i<4;++i){c.update();assert(c._E==bool(p.mPattern[i]));assert(c._C==i+1);}
 c.update();assert(!c._0&&!c._E&&c._C==0&&!c._10);
 c.setPattern(&owner,p,3,true);for(int i=0;i<100;++i){c.update();assert(c._E==bool(p.mPattern[i%4]));assert(c._C==i%4+1);}
 c.clear();assert(!c._0&&!c._4&&!c._8&&!c._C&&!c._E&&!c._10);
 puts("PASS original rumble channel: finite termination, 100 loop frames, owner/priority and clear");
}
