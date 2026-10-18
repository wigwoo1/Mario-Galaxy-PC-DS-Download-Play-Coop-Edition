#include <JSystem/JAudio2/dspproc.hpp>
#include <revolution/types.h>
#include <cassert>
#include <cstdio>
static u32 words[5];static unsigned count;
int DSPSendCommands2(u32* p,u32 n,void(*cb)(u16)){count=n;for(unsigned i=0;i<(n?n:1);++i)words[i]=p[i];if(cb)cb(7);return 1;}
u16 DSP_CreateMap2(u32){return 0x1234;}
extern "C" void OSReport(const char*,...){}
int main(){DsetMixerLevel(2.0f);DsyncFrame2ch(7,0x12345678,0x87654321);assert(count==5&&words[0]==0x82072000&&words[1]==0x12345678&&words[2]==0x87654321&&!words[3]&&!words[4]);DsyncFrame4ch(10,1,2,3,4);assert(count==5&&words[0]==0x820a2000&&words[1]==1&&words[2]==2&&words[3]==3&&words[4]==4);DsetupTable(64,0x1000,0x2000,0x3000,0x4000);assert(count==5&&words[0]==0x81000040&&words[1]==0x1000&&words[4]==0x4000);DsetVARAM(0x76543210);assert(count==2&&words[0]==0x8e000000&&words[1]==0x76543210);DSPReleaseHalt2(0x55);assert(count==0&&words[0]==0x551234);puts("PASS original DSP frame/table/ARAM/halt command words (recording transport fixture)");}
