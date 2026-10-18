#include <Native/GxCommandState.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;
 const unsigned char stream[]={0x61,0x48,0,0xab,0xcd,0x61,0x47,0,0xab,0xcd,0x61,0x52,0,0,0,0x61,0x48,0,0xab,0xcd,0x61,0x45,0,0,2};
 state.consume(stream,sizeof(stream));
 auto events=state.pendingSyncEvents();assert(events.size()==3&&events[0].token==0xabcd&&events[1].token==0xabcd);
 assert(events[0].kind==Native::GxCommandState::SyncEvent::Kind::Token&&events[2].kind==Native::GxCommandState::SyncEvent::Kind::Finish);
 assert(state.pendingCopies().size()==1);
 assert(events[0].sequence<state.pendingCopies()[0].sequence&&state.pendingCopies()[0].sequence<events[1].sequence&&events[1].sequence<events[2].sequence);
 const unsigned char bad[]={0x61,0x48,0,0,7,0x61,0x45};
 bool rejected=false;try{state.consume(bad,sizeof(bad));}catch(const std::length_error&){rejected=true;}
 assert(rejected&&state.pendingSyncEvents().size()==3&&state.bp(0x48)==0xabcd);
 state.clearPendingSyncEvents();assert(state.pendingSyncEvents().empty()&&state.pendingCopies().size()==1);
 const unsigned char zero[]={0x61,0x48,0,0,0};state.consume(zero,sizeof(zero));assert(state.pendingSyncEvents()[0].token==0&&state.pendingSyncEvents()[0].sequence>events[2].sequence);
  Native::GpuResources resources;const unsigned char child[]={0x61,0x48,0,0,2};
 auto address=resources.add(child,sizeof(child));
 unsigned char nested[]={0x61,0x48,0,0,1,0x40,static_cast<unsigned char>(address>>24),static_cast<unsigned char>(address>>16),static_cast<unsigned char>(address>>8),static_cast<unsigned char>(address),0,0,0,5,0x61,0x48,0,0,3};
 Native::GxCommandState nestedState;nestedState.consume(nested,sizeof(nested),&resources);
 const auto& ns=nestedState.pendingSyncEvents();assert(ns.size()==3&&ns[0].token==1&&ns[1].token==2&&ns[2].token==3&&ns[0].sequence<ns[1].sequence&&ns[1].sequence<ns[2].sequence);
 std::puts("PASS repeated GX tokens, finish/copy ordering and malformed-list transactional preservation");
}

