#include <JSystem/JAudio2/dsptask.hpp>
#include <revolution/dsp.h>
#include <revolution/os.h>
#include <cassert>
#include <cstdio>
void DspInitWork();static u32 sent[8];static unsigned count,interrupts,callbacks;static bool busy,enabled=true;
void Dsp_Running_Start();u16 DSP_CreateMap2(u32){return 0;}
extern "C" s32 nativeDspProcessHostAudioCommand(const u32*,u32,u16*){return 0;}
extern "C" u32 DSPCheckMailToDSP(){return busy;}
extern "C" void DSPSendMailToDSP(u32 word){assert(!enabled);sent[count++]=word;}
extern "C" void DSPAssertInt(){++interrupts;}
extern "C" BOOL OSDisableInterrupts(){bool old=enabled;enabled=false;return old;}
extern "C" BOOL OSRestoreInterrupts(BOOL old){bool previous=enabled;enabled=old;return previous;}
extern "C" void OSReport(const char*,...){}
void complete(u16){++callbacks;}
int main(){Dsp_Running_Start();DspInitWork();u32 words[]={0x81000040,0x12345678,0x87654321};assert(DSPSendCommands2(words,3,complete)==1);assert(enabled&&count==4&&sent[0]==3&&sent[1]==words[0]&&sent[3]==words[2]&&interrupts==1&&!callbacks);DspFinishWork(0x8100);assert(callbacks==1);count=0;assert(DSPSendCommands2(words,0,nullptr)==0);assert(count==2&&sent[0]==0&&sent[1]==words[0]&&enabled);busy=true;count=0;assert(DSPSendCommands2(words,3,complete)==-1&&count==0&&enabled&&interrupts==2);busy=false;for(unsigned i=0;i<15;++i){count=0;assert(DSPSendCommands2(words,3,complete)>0);}unsigned beforeInterrupts=interrupts;count=0;assert(DSPSendCommands2(words,3,complete)==-1&&count==0&&interrupts==beforeInterrupts&&enabled);for(unsigned i=0;i<15;++i)DspFinishWork(0x8100);assert(callbacks==16);count=0;assert(DSPSendCommands2(words,3,complete)>0);DspFinishWork(0x8100);assert(callbacks==17);puts("PASS original DSP send mailbox ordering, zero-count command, callback completion and busy rejection (mailbox fixture)");}


