#include <revolution/types.h>
#include <cassert>
#include <cstdio>
void DsyncFrame2(u32,u32,u32);void DsyncFrame3(u32,u32,u32,u32,u32);void Dsp_Update_Request();int Dsp_Running_Check();void Dsp_Running_Start();
static unsigned sent;static u32 words[5];
void DsyncFrame2ch(u32 a,u32 b,u32 c){++sent;words[0]=a;words[1]=b;words[2]=c;words[3]=words[4]=0;}
void DsyncFrame4ch(u32 a,u32 b,u32 c,u32 d,u32 e){++sent;words[0]=a;words[1]=b;words[2]=c;words[3]=d;words[4]=e;}
extern "C" void OSReport(const char*,...){}
int main(){assert(!Dsp_Running_Check());DsyncFrame3(7,1,2,3,4);Dsp_Update_Request();assert(!sent);DsyncFrame2(10,0x12345678,0x87654321);assert(!sent);Dsp_Running_Start();assert(Dsp_Running_Check());Dsp_Update_Request();assert(sent==1&&words[0]==10&&words[1]==0x12345678&&words[2]==0x87654321&&!words[3]&&!words[4]);Dsp_Update_Request();assert(sent==1);DsyncFrame3(7,1,2,3,4);assert(sent==2&&words[3]==3&&words[4]==4);Dsp_Update_Request();assert(sent==2);puts("PASS original deferred latest-frame retention, readiness, single dispatch and direct four-channel dispatch");}
