#include <Game/RhythmLib/AudMePlayer.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){u8 data[]={0,0,0,1,0,0,0,12,0,0,0,20,7,100,3,4,0x12,0x34,6,8,0,0,0,24,'m','e',0};u8 before[sizeof(data)];std::memcpy(before,data,sizeof(data));AudMePlayingParamsHolder holder;holder.setResource(data);assert(holder.mNumEntries==1&&holder.mParams[0].mPriority==7&&holder.mParams[0].mChordVolume==100&&holder.mParams[0]._4==0x1234&&!std::strcmp(holder.mNames[0],"me"));assert(!std::memcmp(before,data,sizeof(data)));holder.setResource(data);assert(holder.mParams[0]._4==0x1234);holder.setResource(nullptr);assert(!holder.mNumEntries&&!holder.mParams&&!holder.mNames);std::puts("PASS native ME params: BE offsets/u16, 64-bit names, unchanged resource, reload and clear");}
