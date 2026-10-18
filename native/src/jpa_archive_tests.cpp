#include "native_disc_archive_loader.hpp"
#include <Native/JpaArchive.hpp>
#include <cassert>
#include <cstdio>
#include <map>
#include <new>

static void* JKRAllocFromHeap(void*,size_t bytes,unsigned alignment){
 if(bytes!=3*sizeof(void*) || alignment<alignof(void*))throw std::runtime_error("Particle callback allocation is too small or misaligned for host pointers");
 return ::operator new(bytes);
}
static void callbackAllocationChecks(){
 using EmitterFunc=void(*)(void*);using ParticleFunc=void(*)(void*,void*);
 void* heap=nullptr;
 unsigned mpCalcEmitterFuncListNum=3,mpCalcParticleFuncListNum=3,mpCalcParticleChildFuncListNum=3,mpDrawEmitterFuncListNum=3,mpDrawEmitterChildFuncListNum=3,mpDrawParticleFuncListNum=3,mpDrawParticleChildFuncListNum=3;
 EmitterFunc *mpCalcEmitterFuncList,*mpDrawEmitterFuncList,*mpDrawEmitterChildFuncList;
 ParticleFunc *mpCalcParticleFuncList,*mpCalcParticleChildFuncList,*mpDrawParticleFuncList,*mpDrawParticleChildFuncList;
 // Actual allocator statements extracted from JPAResource::init, rather than
 // a second copy of their formulas. The allocator boundary checks host size.
#include "jpa_callback_allocations.inc"
 for(void* p:{static_cast<void*>(mpCalcEmitterFuncList),static_cast<void*>(mpDrawEmitterFuncList),static_cast<void*>(mpDrawEmitterChildFuncList),static_cast<void*>(mpCalcParticleFuncList),static_cast<void*>(mpCalcParticleChildFuncList),static_cast<void*>(mpDrawParticleFuncList),static_cast<void*>(mpDrawParticleChildFuncList)})::operator delete(p);
}

static unsigned reference16(const uint8_t* p){return unsigned(p[0])*256+p[1];}
static unsigned reference32(const uint8_t* p){return unsigned(p[0])<<24|unsigned(p[1])<<16|unsigned(p[2])<<8|p[3];}
template<class T>static T host(const std::vector<uint8_t>& b,size_t p){T v;assert(p+sizeof(v)<=b.size());std::memcpy(&v,b.data()+p,sizeof(v));return v;}
int main(int argc,char** argv){
 try{callbackAllocationChecks();}catch(const std::exception& e){std::fprintf(stderr,"FAIL callback allocations: %s\n",e.what());return 22;}
 std::puts("PASS all seven production callback allocation sizes/alignments");
 assert(argc==2);NativeDiscArchiveLoader loader(std::filesystem::u8path(argv[1]));
 auto archive=loader.mount("/ParticleData/Effect.arc");
 const uint8_t* source=nullptr;
 for(const auto& file:archive->files()){
  auto name=file.path.substr(file.path.find_last_of('/')+1);
  for(auto& c:name)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
  if(name=="particles.jpc")source=archive->get(file.path);
 }
 assert(source);size_t size=archive->sizeOf(source);std::vector<uint8_t> original(source,source+size);
 std::printf("Retail particle image: %zu bytes\n",size); auto native=NativeJpa::normalize(source,size);assert(std::memcmp(source,original.data(),size)==0);
 unsigned resources=reference16(source+8),textures=reference16(source+10);assert(resources==3327&&textures==225);
 assert(host<uint16_t>(native,8)==resources&&host<uint16_t>(native,10)==textures);
 size_t pos=16;std::map<std::string,unsigned> blocks;bool found=false;
 for(unsigned r=0;r<resources;++r){
  assert(host<uint16_t>(native,pos)==reference16(source+pos));if(reference16(source+pos)==1351)found=true;
  unsigned count=reference16(source+pos+2);assert(host<uint16_t>(native,pos+2)==count);pos+=8;
  for(unsigned b=0;b<count;++b){
   std::string tag(reinterpret_cast<const char*>(source+pos),4);++blocks[tag];unsigned length=reference32(source+pos+4);
   assert(host<uint32_t>(native,pos)==reference32(source+pos));assert(host<uint32_t>(native,pos+4)==length);
   if(tag=="BEM1"){for(unsigned f=8;f<0x68;f+=4)assert(host<uint32_t>(native,pos+f)==reference32(source+pos+f));for(unsigned f=0x68;f<0x78;f+=2)assert(host<uint16_t>(native,pos+f)==reference16(source+pos+f));}
   if(tag=="BSP1"){assert(host<uint16_t>(native,pos+0x24)==reference16(source+pos+0x24));assert(std::memcmp(native.data()+pos+0x26,source+pos+0x26,8)==0);}
   pos+=length;
  }
 }
 assert(found);pos=reference32(source+12);
 for(unsigned t=0;t<textures;++t){unsigned length=reference32(source+pos+4);assert(std::memcmp(native.data()+pos+12,source+pos+12,20)==0);assert(host<uint16_t>(native,pos+0x22)==reference16(source+pos+0x22));assert(host<uint32_t>(native,pos+0x3c)==reference32(source+pos+0x3c));assert(std::memcmp(native.data()+pos+0x40,source+pos+0x40,length-0x40)==0);pos+=length;}
 for(const auto& item:blocks)std::printf("%s: %u blocks\n",item.first.c_str(),item.second);
 auto rejects=[](const std::vector<uint8_t>& bad){try{NativeJpa::normalize(bad.data(),bad.size());return false;}catch(const std::exception&){return true;}};
 assert(rejects(std::vector<uint8_t>(original.begin(),original.begin()+15)));
 auto bad=original;bad[4]='X';assert(rejects(bad));bad=original;bad[12]=0xff;assert(rejects(bad));
 bad=original;bad[28]=0xff;bad[29]=0xff;bad[30]=0xff;bad[31]=0xff;assert(rejects(bad));
 assert(rejects(std::vector<uint8_t>(original.begin(),original.end()-1)));
 assert(loader.unmount("/ParticleData/Effect.arc"));archive.reset();
 assert(host<uint16_t>(native,8)==3327);
 std::puts("PASS retail JPAC directory, dynamics, color bytes, texture headers/pixels, ownership, immutability and malformed extents");
}
