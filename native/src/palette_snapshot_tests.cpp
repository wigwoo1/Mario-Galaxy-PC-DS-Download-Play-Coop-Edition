#include <Native/GxCommandState.hpp>
#include <cassert>
#include <cstdio>
#include <windows.h>
#include <psapi.h>
static void bp(Native::GxCommandState& state,unsigned reg,unsigned value,const Native::GpuResources& resources){unsigned word=(reg<<24)|value;unsigned char bytes[]={0x61,(unsigned char)(word>>24),(unsigned char)(word>>16),(unsigned char)(word>>8),(unsigned char)word};state.consume(bytes,5,&resources);}
static SIZE_T memory(){PROCESS_MEMORY_COUNTERS_EX info{};assert(GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&info),sizeof(info)));return info.PrivateUsage;}
int main(){
 Native::GxCommandState state;Native::GpuResources resources;
 unsigned char first[32],second[32];for(unsigned i=0;i<32;++i){first[i]=i;second[i]=255-i;}
 auto a=resources.add(first,32),b=resources.add(second,32);
 bp(state,0x64,a>>5,resources);bp(state,0x65,1<<10,resources);
 auto before=memory();std::vector<Native::GxCommandState> snapshots(1024,state);auto increase=memory()-before;
 assert(increase<128*1024*1024); // Full palette duplication would exceed 512 MiB.
 bp(state,0x64,b>>5,resources);bp(state,0x65,1<<10,resources);
 auto updated=state.palette(0,16);for(unsigned i=0;i<32;++i)assert(updated[i]==second[i]);
 for(const auto& snapshot:snapshots){auto old=snapshot.palette(0,16);for(unsigned i=0;i<32;++i)assert(old[i]==first[i]);}
 const unsigned char malformed[]={0x61,0x64,0,0,(unsigned char)(a>>5),0x61,0x65,0,4,0,0xff};
 bool rejected=false;try{state.consume(malformed,sizeof(malformed),&resources);}catch(const std::runtime_error&){rejected=true;}assert(rejected&&state.palette(0,16)==updated);
 std::printf("PASS 1024 palette snapshots: %zu MiB memory increase, independent transfers and rollback\n",size_t(increase/(1024*1024)));
}

