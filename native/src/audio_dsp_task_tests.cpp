#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include <revolution/types.h>
#include <cassert>
#include <cstdio>
void DspInitWork();int DspStartWork(u32,void(*)(u16));
static u32 haltToken;void DSPReleaseHalt2(u32 token){haltToken=token;}
static unsigned calls,reports;static u16 lastSlot;
void complete(u16 slot){++calls;lastSlot=slot;}
extern "C" void OSReport(const char*,...){++reports;}
int main(){JASDsp::releaseHalt(0x12345678);assert(haltToken==0x12345678);DspInitWork();for(unsigned i=0;i<15;++i)assert(DspStartWork((0x100+i)<<16,complete)==i+1);assert(!DspStartWork(0x2000000,complete));assert(reports==1);JASDsp::finishWork(0x999);assert(calls==0&&reports==2);for(unsigned i=0;i<15;++i){JASDsp::finishWork(0x100+i);assert(calls==i+1&&lastSlot==i);}for(unsigned i=0;i<48;++i){assert(DspStartWork((0x300+i)<<16,complete)==((15+i)&15)+1);JASDsp::finishWork(0x300+i);assert(lastSlot==((15+i)&15));}assert(calls==63);assert(DspStartWork(0x7770000,nullptr));JASDsp::finishWork(0x777);assert(calls==63);puts("PASS original DSP task capacity, mismatched completion retention, ordered callbacks and ring wraparound");}

