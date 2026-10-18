#include <Native/GxCommandState.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;
 auto load=[&](unsigned address,const std::vector<u32>& words){std::vector<u8> bytes{0x10};auto emit=[&](u32 w){for(int s=24;s>=0;s-=8)bytes.push_back(w>>s);};emit(((words.size()-1)<<16)|address);for(auto w:words)emit(w);state.consume(bytes.data(),bytes.size());};
 load(0x100a,{0x12345678,0x9abcdef0,0x23456789,0xabcdef01});
 load(0x100e,{1|2|64|(5<<2)|(10<<11)|(2<<7)|512|1024,0,2|(1<<7),1});
 auto rgb=state.lightingChannel(0);assert(rgb.materialVertex&&rgb.enabled&&rgb.ambientVertex&&rgb.lightMask==0xa5&&rgb.diffuse==2&&rgb.attenuation&&rgb.spot);
 auto alpha=state.lightingChannel(0,true);assert(alpha.enabled&&!alpha.materialVertex&&!alpha.ambientVertex&&alpha.diffuse==1&&alpha.lightMask==0);
 assert((state.lightingColor(1,true)==std::array<unsigned,4>{0x9a,0xbc,0xde,0xf0}));
 assert((state.lightingColor(0)==std::array<unsigned,4>{0x23,0x45,0x67,0x89}));
 std::vector<u32> light{0,0,0,0x13579bdf};for(float v:{1.f,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f,-1.f,-2.f,-3.f}){u32 bits;std::memcpy(&bits,&v,4);light.push_back(bits);}
 load(0x670,light);auto actual=state.light(7);assert((actual.color==std::array<unsigned,4>{0x13,0x57,0x9b,0xdf}));assert((actual.angular==std::array<float,3>{1,2,3}));assert((actual.distance==std::array<float,3>{4,5,6}));assert((actual.position==std::array<float,3>{7,8,9}));assert((actual.direction==std::array<float,3>{-1,-2,-3}));
 bool rejected=false;try{state.light(0);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
 load(0x100e,{3<<7});rejected=false;try{state.lightingChannel(0);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 std::puts("PASS GX lighting state: separate RGB/alpha, eight-bit light mask, colors and complete light object");
}
