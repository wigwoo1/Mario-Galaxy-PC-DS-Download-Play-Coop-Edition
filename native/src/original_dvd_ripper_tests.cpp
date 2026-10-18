#include "../build/AudioArcInspectorFixture.hpp"
#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <JSystem/JKernel/JKRDvdRipper.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRDecomp.hpp>
#include <JSystem/JUtility/JUTException.hpp>
#include <cassert>
#include <cstdio>
#include <cstdlib>

static bool failRefill=false;static unsigned failures=0,retraces=0;
extern "C" void VIWaitForRetrace(){++retraces;}
extern "C" s32 nativeTestDVDReadPrio(DVDFileInfo* info,void* dst,s32 len,s32 off,s32 priority){if(failRefill&&off>0){failRefill=false;++failures;return -1;}return DVDReadPrio(info,dst,len,off,priority);}
// The retail Yaz0 path decompresses directly from DVD; this unused Yay0 sink fails loudly.
bool JKRDecomp::orderSync(u8*,u8*,u32,u32){std::abort();}
int main(int argc,char**argv){assert(argc==2);auto path=std::filesystem::u8path(argv[1]);WiiAssets reference(path);auto compressed=reference.readFile("/AudioRes/SMR.szs");assert(!std::memcmp(compressed.data(),"Yaz0",4));auto expected=decodeYaz0(compressed);assert(expected.size()==596320);assert(nativeBindDvd(path));alignas(32) static u8 storage[2097152];JKRExpHeap heap(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&heap;JKRHeap::sSystemHeap=&heap;heap.becomeCurrentHeap();auto initial=heap.getTotalFreeSize();for(int i=0;i<4;++i){u32 length=0;int compression=0;void* data=JKRDvdToMainRam("/AudioRes/SMR.szs",nullptr,EXPAND_SWITCH_UNKNOWN1,0,&heap,JKRDvdRipper::ALLOC_DIRECTION_BACKWARD,0,&compression,&length);assert(data);assert(compression==2);assert(length==expected.size());assert(!std::memcmp(data,expected.data(),length));Inspector inspector;inspector.base=static_cast<const u8*>(data);inspector.length=length;assert(inspector.parse(data,length));assert(inspector.ws==48&&inspector.banks==48&&inspector.bst==1&&inspector.names==1&&inspector.waves==1796&&inspector.oscillators==334);JKRHeap::free(data,&heap);assert(heap.check());assert(heap.getTotalFreeSize()==initial);}for(bool retry:{false,true}){JKRDvdRipper::errorRetry=retry;failRefill=true;failures=0;retraces=0;u32 length=0;void* data=JKRDvdToMainRam("/AudioRes/SMR.szs",nullptr,EXPAND_SWITCH_UNKNOWN1,0,&heap,JKRDvdRipper::ALLOC_DIRECTION_BACKWARD,0,nullptr,&length);assert(failures==1);if(retry){assert(data&&length==expected.size());assert(!std::memcmp(data,expected.data(),length));assert(retraces==1);JKRHeap::free(data,&heap);}else{assert(!data);assert(retraces==0);}assert(heap.check());assert(heap.getTotalFreeSize()==initial);}JKRDvdRipper::errorRetry=true;for(u32 offset:{0u,8192u,u32(expected.size()-17),u32(expected.size()),u32(expected.size()+1)}){alignas(32) u8 destination[96];std::memset(destination,0xA5,sizeof(destination));u32 length=0;void* result=JKRDvdToMainRam("/AudioRes/SMR.szs",destination,EXPAND_SWITCH_UNKNOWN1,32,&heap,JKRDvdRipper::ALLOC_DIRECTION_BACKWARD,offset,nullptr,&length);if(offset>expected.size()){assert(!result);}else{assert(result==destination);auto count=std::min<size_t>(32,expected.size()-offset);assert(length==count);assert(!std::memcmp(destination,expected.data()+offset,count));for(size_t i=count;i<sizeof(destination);++i)assert(destination[i]==0xA5);}assert(heap.check());assert(heap.getTotalFreeSize()==initial);}for(u32 capacity:{512u,2048u}){alignas(32) u8 lowStorage[2048]{};JKRExpHeap low(lowStorage,capacity,&heap,false);auto lowInitial=low.getTotalFreeSize();JKRHeap::sSystemHeap=&low;u32 length=123;void* result=JKRDvdToMainRam("/AudioRes/SMR.szs",nullptr,EXPAND_SWITCH_UNKNOWN1,32,&heap,JKRDvdRipper::ALLOC_DIRECTION_BACKWARD,capacity==512?0:8192,nullptr,&length);assert(!result&&length==0);assert(low.check()&&low.getTotalFreeSize()==lowInitial);assert(heap.check()&&heap.getTotalFreeSize()==initial);JKRHeap::sSystemHeap=&heap;}u32 recoveryLength=0;void* recovery=JKRDvdToMainRam("/AudioRes/SMR.szs",nullptr,EXPAND_SWITCH_UNKNOWN1,32,&heap,JKRDvdRipper::ALLOC_DIRECTION_BACKWARD,8192,nullptr,&recoveryLength);assert(recovery&&recoveryLength==32);assert(!std::memcmp(recovery,expected.data()+8192,32));JKRHeap::free(recovery,&heap);assert(heap.getTotalFreeSize()==initial);nativeReleaseDvd();std::puts("PASS original DVD ripper: retail SMR Yaz0 bytes, 596320-byte returned extent, four loads with bounded original audio dispatch (48 banks/48 wave systems/1796 waves) plus failed-refill cleanup and one-retry recovery and bounded offset reads and low-memory recovery with restored heap");}

extern "C" void DCStoreRangeNoSync(void*,u32){} // CPU-only fixture: no GPU resources participate.

extern "C" void DCInvalidateRange(void*,u32){} // CPU-only fixture.






