#include <Native/GxCommandState.hpp>
#include <initializer_list>
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>
u32 word(const u8* p){assert(p[0]==0x61);return u32(p[1])<<24|u32(p[2])<<16|u32(p[3])<<8|p[4];}
int main(){
 u8 bytes[64]={};GDLObj dl;GDInitGDLObj(&dl,bytes,64);GDSetCurrent(&dl);
 GXSetCopyClear(GXColor{0x12,0x34,0x56,0x78},0xabcdef);
 Native::GxCommandState state;state.consume(bytes,GDGetCurrOffset());auto clear=state.copyClear();assert((clear.rgba==std::array<std::uint8_t,4>{0x12,0x34,0x56,0x78})&&clear.depth==0xabcdef);
 assert(GDGetCurrOffset()==15);assert(word(bytes)==0x4f007812&&word(bytes+5)==0x50003456&&word(bytes+10)==0x51abcdef);
 GDInitGDLObj(&dl,bytes,64);GXSetCopyFilter(0,nullptr,0,nullptr);
 assert(GDGetCurrOffset()==30);for(unsigned i=0;i<4;i++)assert(word(bytes+i*5)==((i+1)<<24|0x666666));
 assert(word(bytes+20)==(0x53000000|21u<<12|22u<<18));assert(word(bytes+25)==0x54000015);
 state.consume(bytes,GDGetCurrOffset());auto defaults=state.copyFilter();for(auto sample:defaults.samples)assert(sample[0]==6&&sample[1]==6);assert((defaults.coefficients==std::array<unsigned,7>{0,0,21,22,21,0,0}));
 u8 samples[12][2];for(unsigned i=0;i<12;i++){samples[i][0]=i;samples[i][1]=15-i;}
 u8 coefficients[7]={1,2,3,4,5,6,7};GDInitGDLObj(&dl,bytes,64);GXSetCopyFilter(1,samples,1,coefficients);
 for(unsigned i=0;i<4;i++){u32 expected=(i+1)<<24;for(unsigned j=0;j<3;j++){expected|=u32(samples[i*3+j][0])<<(j*8);expected|=u32(samples[i*3+j][1])<<(j*8+4);}assert(word(bytes+i*5)==expected);}
 assert(word(bytes+20)==(0x53000000|1|2u<<6|3u<<12|4u<<18));assert(word(bytes+25)==(0x54000000|5|6u<<6|7u<<12));
 state.consume(bytes,GDGetCurrOffset());auto filter=state.copyFilter();for(unsigned i=0;i<12;i++)assert(filter.samples[i][0]==samples[i][0]&&filter.samples[i][1]==samples[i][1]);for(unsigned i=0;i<7;i++)assert(filter.coefficients[i]==coefficients[i]);
 for(unsigned size: {14u,29u}){for(auto& b:bytes)b=0xa5;GDInitGDLObj(&dl,bytes,size);bool caught=false;try{if(size==14)GXSetCopyClear(GXColor{},0);else GXSetCopyFilter(0,nullptr,0,nullptr);}catch(const std::length_error&){caught=true;}assert(caught&&GDGetCurrOffset()==0);for(auto b:bytes)assert(b==0xa5);}
 GDInitGDLObj(&dl,bytes,64);GDWriteBPCmd(0x49000000|(17u)|(23u<<10));GDWriteBPCmd(0x4a000000|607u|(447u<<10));GDWriteBPCmd(0x4d000026);GDWriteBPCmd(0x4b000100);GDWriteBPCmd(0x52004000);GDWriteBPCmd(0x4b000200);GDWriteBPCmd(0x52000000);
 state.consume(bytes,GDGetCurrOffset());assert(state.pendingCopies().size()==2);
 assert(state.pendingCopies()[0].registers[0x4b]==0x100&&state.pendingCopies()[0].registers[0x52]==0x4000);
 assert(state.pendingCopies()[1].registers[0x4b]==0x200&&state.pendingCopies()[1].registers[0x52]==0);
 const auto& copy=state.pendingCopies()[0];assert(copy.left()==17&&copy.top()==23&&copy.width()==608&&copy.height()==448&&copy.destinationAddress()==8192&&copy.strideBytes()==1216&&copy.display()&&!copy.clear());assert(!state.pendingCopies()[1].display());
 assert(state.pendingCopies()[0].registers[0x4f]==0x7812);
 // A later malformed command must roll back any queued copy and state writes.
 GDInitGDLObj(&dl,bytes,64);GDWriteBPCmd(0x52004000);GDWrite_u8(0xff);
 bool malformed=false;try{state.consume(bytes,GDGetCurrOffset());}catch(const std::runtime_error&){malformed=true;}
 assert(malformed&&state.pendingCopies().size()==2&&state.bp(0x52)==0);
 state.clearPendingCopies();assert(state.pendingCopies().empty());
 GDSetCurrent(nullptr);std::puts("PASS original copy clear, AA/default sample/filter commands and atomic capacity rejection");
}





