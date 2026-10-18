#include <Native/GxCommandState.hpp>
#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
#include <stdexcept>
int main(){
 u8 bytes[32]{};GDLObj dl;Native::GpuResources resources;Native::GxCommandState state;
 u8 matrices[96]{};
 // Two packed 3x4 rows of matrices; indexed source 1 contains a translation.
 const float values[12]={2,0,0,3,0,4,0,-1,0,0,5,2};
 for(unsigned i=0;i<12;++i){u32 v;std::memcpy(&v,&values[i],4);for(unsigned j=0;j<4;++j)matrices[48+i*4+j]=v>>(24-j*8);}
 const auto address=resources.add(matrices,sizeof(matrices));
 GDInitGDLObj(&dl,bytes,sizeof(bytes));GDSetCurrent(&dl);
 GDWriteCPCmd(0xae,address);GDWriteCPCmd(0xbe,48);
 state.consume(bytes,GDGetCurrOffset(),&resources);
 for(auto type:{GX_MTX2x4,GX_MTX3x4})for(u32 id:{30u,64u,67u,125u}){
  GDInitGDLObj(&dl,bytes,sizeof(bytes));GXLoadTexMtxIndx(1,id,type);
  const u32 destination=id>=64?0x500+(id-64)*4:id*4;
  const u32 descriptor=((type==GX_MTX2x4?7:11)<<12)|destination;
  assert(GDGetCurrOffset()==5&&bytes[0]==0x30&&bytes[1]==0&&bytes[2]==1&&bytes[3]==(descriptor>>8)&&bytes[4]==(descriptor&255));
  state.consume(bytes,5,&resources);
 }
 assert((state.transformTexture(30,{.25f,.5f,1},true)==std::array<float,3>{3.5f,1,7}));
 assert((state.transformTexture(3,{.25f,.5f,1},true,true)==std::array<float,3>{3.5f,1,7}));
 for(u32 id:{61u,63u,126u,0xffffffffu}){
  GDInitGDLObj(&dl,bytes,sizeof(bytes));bool rejected=false;try{GXLoadTexMtxIndx(0,id,GX_MTX3x4);}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 }
 GDInitGDLObj(&dl,bytes,sizeof(bytes));bool rejected=false;try{GXLoadTexMtxIndx(0,30,GXTexMtxType(2));}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 for(unsigned size=1;size<5;++size){for(auto&b:bytes)b=0xa5;GDInitGDLObj(&dl,bytes,size);rejected=false;try{GXLoadTexMtxIndx(1,30,GX_MTX3x4);}catch(const std::length_error&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);for(auto b:bytes)assert(b==0xa5);}
 GDInitGDLObj(&dl,bytes,sizeof(bytes));GXLoadTexMtxIndx(2,30,GX_MTX3x4);rejected=false;try{state.consume(bytes,5,&resources);}catch(const std::out_of_range&){rejected=true;}assert(rejected);
 assert((state.transformTexture(30,{.25f,.5f,1},true)==std::array<float,3>{3.5f,1,7}));
 GDSetCurrent(nullptr);puts("PASS indexed texture matrix commands: ST/STQ, post destinations, nonzero source, bounded writes and invalid-source rollback");
}
