#include "native_runtime_api.h"
#include <windows.h>
#include <cassert>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstring>
#include <cmath>
#include <cstdio>
template<class T>T entry(HMODULE dll,const char* name){auto p=GetProcAddress(dll,name);assert(p);return reinterpret_cast<T>(p);}
static unsigned be32(const std::vector<uint8_t>& b,size_t p){assert(p+4<=b.size());return unsigned(b[p])<<24|unsigned(b[p+1])<<16|unsigned(b[p+2])<<8|b[p+3];}
static unsigned be16(const std::vector<uint8_t>& b,size_t p){assert(p+2<=b.size());return unsigned(b[p])<<8|b[p+1];}
static float value(const std::vector<uint8_t>& b,size_t p){auto bits=be32(b,p);float f;std::memcpy(&f,&bits,4);return f;}
int main(int argc,char** argv){assert(argc==4);auto dll=LoadLibraryA(argv[1]);assert(dll);
auto version=entry<decltype(&galaxy_runtime_version)>(dll,"galaxy_runtime_version");auto error=entry<decltype(&galaxy_runtime_error)>(dll,"galaxy_runtime_error");auto create=entry<decltype(&galaxy_runtime_create)>(dll,"galaxy_runtime_create");auto destroy=entry<decltype(&galaxy_runtime_destroy)>(dll,"galaxy_runtime_destroy");auto load=entry<decltype(&galaxy_runtime_load_terrain)>(dll,"galaxy_runtime_load_terrain");auto tick=entry<decltype(&galaxy_runtime_tick)>(dll,"galaxy_runtime_tick");auto bind=entry<decltype(&galaxy_runtime_bind)>(dll,"galaxy_runtime_bind");assert(version()==1);
std::ifstream k(argv[2],std::ios::binary),m(argv[3],std::ios::binary);assert(k&&m);std::vector<uint8_t> kcl((std::istreambuf_iterator<char>(k)),{}),metadata((std::istreambuf_iterator<char>(m)),{});
float matrix[]={1,0,0,0,0,1,0,0,0,0,1,0};auto prism=be32(kcl,8)+16;auto positionOffset=be32(kcl,0)+12*be16(kcl,prism+4),normalOffset=be32(kcl,4)+12*be16(kcl,prism+6);
float contactPosition[3],gravity[3],toward[3];for(int j=0;j<3;++j){float normal=value(kcl,normalOffset+4*j);contactPosition[j]=value(kcl,positionOffset+4*j)+normal*5;gravity[j]=-normal;toward[j]=-normal*2;}
for(int repeat=0;repeat<2;++repeat){void* scene=create(2);assert(scene);assert(!create(2)&&std::strlen(error())>0);assert(!load(scene,kcl.data(),8,metadata.data(),metadata.size(),matrix,"bad",1));assert(load(scene,kcl.data(),kcl.size(),metadata.data(),metadata.size(),matrix,"DLL terrain",1));assert(tick(scene));
float position[]={1e7f,1e7f,1e7f},down[]={0,-1,0},movement[]={7,-3,5},result[]={123,123,123};uint32_t contacts=99;
assert(bind(scene,position,down,movement,10,result,&contacts));assert(contacts==0);for(int j=0;j<3;++j)assert(std::abs(result[j]-movement[j])<.001f);assert(std::strlen(error())==0);
assert(bind(scene,contactPosition,gravity,toward,10,result,&contacts));assert(contacts!=0);for(float v:result)assert(std::isfinite(v));
float zero[]={0,0,0};assert(!bind(scene,position,zero,movement,10,result,&contacts));assert(std::strlen(error())>0);assert(destroy(scene));assert(!tick(scene));}
assert(FreeLibrary(dll));std::puts("PASS native runtime DLL: exports, scene recreation, terrain load, Binder contact/free motion and errors");}
