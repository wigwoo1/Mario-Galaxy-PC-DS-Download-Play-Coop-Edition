#include <Native/DiffuseLighting.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;
 auto load=[&](unsigned address,const std::vector<u32>& words){std::vector<u8> bytes{0x10};auto emit=[&](u32 w){for(int s=24;s>=0;s-=8)bytes.push_back(w>>s);};emit(((words.size()-1)<<16)|address);for(auto w:words)emit(w);state.consume(bytes.data(),bytes.size());};
 load(0x100a,{0x20202020});load(0x100c,{0x80808080});load(0x100e,{0,0,1});
 auto evaluate=[&](std::array<float,3> n){return Native::evaluateDiffuseLighting(state,0,{0,0,0},n,{13,29,71,149});};
 assert((evaluate({0,0,1})==std::array<unsigned,4>{128,128,128,149}));
 std::vector<u32> light{0,0,0,0x60606060};for(float v:{1.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,10.f,0.f,0.f,1.f}){u32 bits;std::memcpy(&bits,&v,4);light.push_back(bits);}load(0x600,light);
 load(0x100e,{2|4|(2<<7)});assert((evaluate({0,0,1})==std::array<unsigned,4>{64,64,64,149}));
 assert((evaluate({0,0,-1})==std::array<unsigned,4>{16,16,16,149}));
 load(0x100e,{2|4|(1<<7)});assert((evaluate({0,0,-1})==std::array<unsigned,4>{0,0,0,149}));
 load(0x100e,{2|4});assert((evaluate({0,0,-1})==std::array<unsigned,4>{64,64,64,149}));
 auto coefficients=[&](std::array<float,3> angular,std::array<float,3> distance){
  std::vector<u32> words;for(auto vector:{angular,distance})for(float v:vector){u32 bits;std::memcpy(&bits,&v,4);words.push_back(bits);}load(0x604,words);
 };
 coefficients({1,0,0},{1,.1f,0});load(0x100e,{2|4|512|1024});
 assert((evaluate({0,0,1})==std::array<unsigned,4>{40,40,40,149}));
 coefficients({0,1,0},{2,0,0});load(0x100e,{2|4|512});
 assert((evaluate({0,0,1})==std::array<unsigned,4>{40,40,40,149}));
 assert((evaluate({0,0,-1})==std::array<unsigned,4>{16,16,16,149}));
 coefficients({0,1,0},{2,0,0});load(0x100e,{2|4|512|(2<<7)});
 assert((evaluate({0,0,1})==std::array<unsigned,4>{64,64,64,149}));
 coefficients({1,0,0},{0,0,0});bool rejected=false;try{evaluate({0,0,1});}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 std::puts("PASS GX spot/specular attenuation: distance falloff, facing, coefficient normalization and singular rejection");
 std::puts("PASS GX diffuse lighting: register/vertex material, independent alpha, none/signed/clamped diffuse and saturation");
}

