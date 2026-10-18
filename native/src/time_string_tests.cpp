#include "Game/Util/StringUtil.hpp"
#include "Game/Screen/ReplaceTagProcessor.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include <cassert>
#include <cstdarg>
#include <cstring>
#include <cstdio>
static const char* message;
static int values[4], count, extent, race;
namespace MR {
const wchar_t* getGameMessageDirect(const char* id) {message=id;return L"fixture";}
u32 getRaceBestTime(int id){race=id;return 3661;}
u32 getRaceCurrentTime(){return 59;}
}
namespace ReplaceTagFunction {
u32 ReplaceArgs(wchar_t*, s32 length, const wchar_t* format, ...) {
 extent=length;count=std::strcmp(message,"System_Time000")==0?4:3;
 va_list args;va_start(args,format);for(int i=0;i<count;i++)values[i]=va_arg(args,int);va_end(args);return 0;
}
}
int main(){
 wchar_t out[16]{};
 const unsigned frames[]={0,1,59,60,3599,3600,215999,216000,0xffffffffu};
 for(unsigned f:frames){
 MR::makeClearTimeString(out,f);assert(extent==9&&count==4&&values[0]==0);
 assert(values[1]==(f>=216000?59:int(f/3600)));
 assert(values[2]==(f>=216000?59:int(f/60%60)));
 assert(values[3]==(f>=216000?99:int(f%60*100/60)));
 MR::makeMinuteAndSecondString(out,f);assert(extent==16&&count==3&&values[0]==0);
 assert(values[1]==(f>=216000?59:int(f/3600)));
 assert(values[2]==(f>=216000?59:int(f%3600/60)));
 }
 MR::makeRaceBestTimeString(out,7);assert(race==7&&values[1]==1&&values[2]==1&&values[3]==1);
 MR::makeRaceCurrentTimeString(out);assert(values[1]==0&&values[2]==0&&values[3]==98);
 std::puts("PASS original time formatting: nine frame boundaries, saturation, race sources; replacement callback fixture only");
}

