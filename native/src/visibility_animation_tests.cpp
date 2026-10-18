#include "native_visibility_animation.hpp"
#include <cassert>
#include "assets.hpp"
#include <cstdio>
int main(int argc,char** argv){
 std::vector<u8> bytes(64);std::memcpy(bytes.data(),"J3D1bva1",8);
 auto w32=[&](int o,unsigned v){for(int i=0;i<4;i++)bytes[o+i]=u8(v>>(24-i*8));};
 auto w16=[&](int o,unsigned v){bytes[o]=u8(v>>8);bytes[o+1]=u8(v);};
 w32(8,64);w32(12,1);std::memcpy(bytes.data()+32,"VAF1",4);w32(36,32);
 bytes[40]=2;w16(42,3);w16(44,1);w16(46,3);w32(48,24);w32(52,28);w16(56,3);w16(58,0);
 bytes[60]=1;bytes[61]=0;bytes[62]=1;
 auto valid=bytes;
 auto animation=Native::createVisibilityAnimation(bytes.data(),bytes.size());
 bytes.assign(64,0);bytes.clear();bytes.shrink_to_fit();
 for(float f:{-3.f,0.f,.49f,.5f,1.f,1.5f,3.f,100.f}) {
  animation->mFrame=f;u8 visibility=9;animation->getVisibility(0,&visibility);
  assert(visibility==((f>=.5f&&f<1.5f)?0:1));
 }
 assert(animation->mAttribute==2&&animation->mFrameMax==3);
 unsigned rejected=0;
 for(int kind=0;kind<5;kind++){
  bytes=valid;
  if(kind==0)bytes.resize(31);
  if(kind==1)bytes[39]=0xff;
  if(kind==2)bytes[51]=0xff;
  if(kind==3){bytes[56]=0;bytes[57]=0;}
  if(kind==4)bytes[59]=3;
  try{auto bad=Native::createVisibilityAnimation(bytes.data(),bytes.size());}
  catch(const std::invalid_argument&){++rejected;}
 }
 assert(rejected==5);
 if(argc==2){
  WiiAssets disc(std::filesystem::u8path(argv[1]));
  std::vector<u8> archiveBytes;std::string selected;
  for(const auto& entry:disc.files){
   if(entry.path.find("/ObjectData/")!=0||entry.path.size()<4||entry.path.substr(entry.path.size()-4)!=".arc"||entry.size>4*1024*1024)continue;
   auto candidate=disc.readFile(entry.path);Rarc probe(candidate);bool found=false;
   for(const auto& f:probe.files){auto b=probe.readFile(f.path);if(b.size()>=8&&!std::memcmp(b.data(),"J3D1bva1",8)){found=true;break;}}
   if(found){archiveBytes=std::move(candidate);selected=entry.path;break;}
  }
  assert(!archiveBytes.empty());std::printf("Retail visibility archive: %s\n",selected.c_str());Rarc archive(archiveBytes);
  size_t files=0,samples=0;
  for(const auto& file:archive.files){
   auto source=archive.readFile(file.path);
   if(source.size()<8||std::memcmp(source.data(),"J3D1bva1",8))continue;
   auto a=Native::createVisibilityAnimation(source.data(),source.size());auto raw=source;
   source.clear();source.shrink_to_fit();
   auto r16=[&](size_t o){return unsigned(raw.at(o))<<8|raw.at(o+1);};
   auto r32=[&](size_t o){return unsigned(raw.at(o))<<24|unsigned(raw.at(o+1))<<16|unsigned(raw.at(o+2))<<8|raw.at(o+3);};
   size_t block=32;while(std::memcmp(raw.data()+block,"VAF1",4))block+=r32(block+4);
   for(unsigned j=0;j<a->mUpdateMaterialNum;j++){
    size_t table=block+r32(block+16)+j*4;int count=r16(table);unsigned offset=r16(table+2);
    for(float f:{-2.f,0.f,.49f,.5f,1.5f,float(count),float(count+10)}){
     a->mFrame=f;u8 value=99;a->getVisibility(j,&value);
     int frame=int(f+.5f);frame=std::max(0,std::min(count-1,frame));
     assert(value==raw.at(block+r32(block+20)+offset+frame));samples++;
    }
   }
   files++;
  }
  assert(files>0);std::printf("PASS retail BVA: %zu files, %zu original sampling comparisons; owned tables after source disposal\n",files,samples);
 }
 std::puts("PASS owned BVA conversion: endian fields, source disposal, original rounding and end clamps");
}


