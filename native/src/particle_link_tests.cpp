#define NOMINMAX
#include <windows.h>
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <Game/Effect/SingleEmitter.hpp>
#include <Game/Effect/EffectSystemUtil.hpp>
#include <cassert>
#include <cstdio>
#include <new>
int main(){
 JPABaseEmitter base;ParticleEmitter particle;particle.mEmitter=&base;
 base.setUserWork(0);base.mStatus=0x12345678;
 for(std::uintptr_t address:{std::uintptr_t(0x10000000000ull),std::uintptr_t(0x10080000000ull)}){
  void* storage=VirtualAlloc(reinterpret_cast<void*>(address),4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
  assert(storage==reinterpret_cast<void*>(address));
  auto* single=new(storage)SingleEmitter;
  for(unsigned cycle=0;cycle<3;++cycle){
   single->link(&particle);
   assert(single->mEmitter==&particle);
   if(MR::Effect::getLinkSingleEmitter(&base)!=single){std::fprintf(stderr,"FAIL particle link truncated: expected=%p actual=%p storedBytes=%zu\n",single,MR::Effect::getLinkSingleEmitter(&base),sizeof(base.mpUserWork));return 1;}
   // The real cleanup writes through this pointer. It must target the original object.
   MR::Effect::getLinkSingleEmitter(&base)->mEmitter=nullptr;
   assert(single->mEmitter==nullptr&&base.mStatus==0x12345678&&base.mAlivePtclBase.getNum()==0);
   single->link(&particle);single->unlink();
   assert(single->mEmitter==nullptr&&MR::Effect::getLinkSingleEmitter(&base)==nullptr);
  }
  single->~SingleEmitter();assert(VirtualFree(storage,0,MEM_RELEASE));
 }
 std::printf("PASS original particle link/unlink and cleanup writes at 1 TiB, including low-word sign bit; storage=%zu emitter=%zu\n",sizeof(base.mpUserWork),sizeof(base));
}
