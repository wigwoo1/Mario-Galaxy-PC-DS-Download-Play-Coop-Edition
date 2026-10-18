#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASDSPChannel.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <cassert>
#include <cstdlib>
#include <cstdio>
JKRSolidHeap* JASDram=nullptr;
void* JKRHeap::alloc(u32 s,int,JKRHeap*) {return std::malloc(s);}
void JKRHeap::free(void* p,JKRHeap*) {std::free(p);}
namespace JASDsp {
TChannel backend[64]{}; unsigned flushes=0,starts=0,stops=0,forced=0;
TChannel* getDSPHandle(int i){return &backend[i];}
void releaseHalt(u32){}
void TChannel::init(){mIsFinished=0;}
void TChannel::playStart(){mIsActive=1;++starts;}
void TChannel::playStop(){mIsActive=0;++stops;}
void TChannel::forceStop(){++forced;}
void TChannel::replyFinishRequest(){mIsFinished=0;}
bool TChannel::isFinish() const{return mIsFinished!=0;}
void TChannel::flush(){++flushes;}
}
struct Calls {unsigned play=0,start=0,stop=0,drop=0;};
s32 callback(u32 event,JASDsp::TChannel*,void* p){auto& c=*static_cast<Calls*>(p);switch(event){case 0:++c.play;break;case 1:++c.start;break;case 2:++c.stop;return -1;case 3:++c.drop;break;}return 0;}
int main(){
 JASDSPChannel channels[64];JASDSPChannel::sDspChannels=channels;
 for(int i=0;i<64;++i)channels[i].mChannel=&JASDsp::backend[i];
 assert(JASDSPChannel::getNumFree()==64);Calls calls[64];
 for(int i=0;i<64;++i){auto* c=JASDSPChannel::alloc(20,callback,&calls[i]);assert(c==&channels[i]);c->start();c->updateProc();assert(calls[i].start==1);}
 assert(JASDSPChannel::getNumUse()==64);assert(!JASDSPChannel::alloc(19,callback,&calls[0]));
 channels[3].field_0xc=8;channels[5].field_0xc=12;
 assert(JASDSPChannel::getLowestChannel(20)==&channels[5]);
 Calls replacement;auto* stolen=JASDSPChannel::alloc(21,callback,&replacement);
 assert(stolen==&channels[5]&&calls[5].drop==1&&stolen->getStatus()==JASDSPChannel::STATUS_DROP);
 stolen->start();stolen->updateProc();assert(JASDsp::forced==1&&replacement.start==0);
 stolen->mChannel->mIsFinished=1;stolen->updateProc();assert(stolen->getStatus()==JASDSPChannel::STATUS_INACTIVE);
 stolen->updateProc();assert(replacement.start==1);stolen->updateProc();assert(replacement.play==1);
 stolen->mChannel->mIsFinished=1;stolen->updateProc();assert(replacement.stop==1&&stolen->mPriority==-1);
 assert(JASDSPChannel::getNumFree()==1);assert(JASDSPChannel::getNumUse()==63);
 assert(JASDSPChannel::killActiveChannel()==1);assert(JASDSPChannel::getNumBreak()==1);
 puts("PASS original DSP channel capacity, priority/age selection, replacement handshake, callbacks and counts (DSP transport fixture).");
 JASDSPChannel::sDspChannels=nullptr;
}
