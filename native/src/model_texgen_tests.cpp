#include "native_model_projection.hpp"
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;
 auto load=[&](unsigned address,const std::vector<u32>& words){std::vector<u8> b{0x10};auto emit=[&](u32 v){for(int s=24;s>=0;s-=8)b.push_back(v>>s);};emit(((words.size()-1)<<16)|address);for(auto v:words)emit(v);state.consume(b.data(),b.size());};
 auto matrix=[&](unsigned address,const std::vector<float>& values){std::vector<u32> words;for(float v:values){u32 bits;std::memcpy(&bits,&v,4);words.push_back(bits);}load(address,words);};
 NativeShapeVertex vertex{};vertex.objectPosition={2,3,4};vertex.position={200,300,400};vertex.normal={0,3,4};vertex.hasNormal=true;vertex.texCoord0={.25f,.5f,0};vertex.hasTexCoord0=true;vertex.textureMatrices[0]=30;
 matrix(120,{2,0,0,1,0,3,0,2,0,0,4,3});
 load(0x1040,{5<<7});assert((generateModelTexCoord(state,vertex,0)==std::array<float,3>{1.5f,3.5f,1}));
 load(0x1040,{4|2});assert((generateModelTexCoord(state,vertex,0)==std::array<float,3>{5,11,19}));
 load(0x1040,{(1<<7)|4|2});assert((generateModelTexCoord(state,vertex,0)==std::array<float,3>{1,11,19}));
 vertex.textureMatrices[0]=60;load(0x1040,{(1<<7)|4|2});load(0x1012,{1});load(0x1050,{256});
 matrix(0x500,{1,0,0,2,0,1,0,3,0,0,1,4});auto normalized=generateModelTexCoord(state,vertex,0);
 assert(std::fabs(normalized[0]-2)<.00001f&&std::fabs(normalized[1]-3.6f)<.00001f&&std::fabs(normalized[2]-4.8f)<.00001f);
 load(0x1040,{1<<4});bool rejected=false;try{generateModelTexCoord(state,vertex,0);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 load(0x1012,{0});load(0x1041,{5<<7});vertex.textureMatrices[1]=60;
 std::vector<u8> bp{0x61,0x28,0,0,0x48};state.consume(bp.data(),bp.size());
 TProj3f projection{};for(unsigned i=0;i<4;++i)projection.mMtx[i][i]=1;
 auto projected=projectModelTriangles({vertex},{{0,0,0}},projection,{1,1,1,1},&state);
 assert(projected.size()==3);for(auto& v:projected)assert(v.extraUv[0][0]==.25f&&v.extraUv[0][1]==.5f&&v.extraUv[0][2]==1);
 std::puts("PASS model projection adapter generates authored active UV channel 1");
 std::puts("PASS regular GX texgen: authored position/normal/UV0, AB11/STQ, dual normalization and post matrix");
}

