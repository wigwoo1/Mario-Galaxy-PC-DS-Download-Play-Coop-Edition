#include "native_jkr_aram_port.hpp"
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRDecomp.hpp>
#include <JSystem/JKernel/JKRAramPiece.hpp>
#include <cstring>
#include <stdexcept>
namespace {std::mutex transportMutex;std::shared_ptr<NativeAramMemory> transport;uint64_t allocationTop=0;bool initialized=false;}
bool nativeBindAramTransport(std::shared_ptr<NativeAramMemory> memory){std::lock_guard<std::mutex> lock(transportMutex);if(!memory || transport)return false;transport=std::move(memory);initialized=false;allocationTop=0;return true;}
void nativeReleaseAramTransport(){std::lock_guard<std::mutex> lock(transportMutex);transport.reset();initialized=false;allocationTop=0;}
void JKRAramPiece::sendCommand(JKRAMCommand* command){startDMA(command);}
void JKRAramPiece::startDMA(JKRAMCommand* command){
 if(!command)throw std::invalid_argument("Null ARAM command");
 std::unique_lock<std::mutex> lock(transportMutex);auto memory=transport;
 if(!memory)throw std::logic_error("Native ARAM transport is unbound");
 unsigned direction=unsigned(command->mTransferDirection);
 // Original direction 0 is main RAM -> ARAM; direction 1 reverses it.
 uint32_t cpu=direction==0?command->mSrc:command->mDst;
 uint32_t aram=direction==0?command->mDst:command->mSrc;
 if(!memory->transfer(direction,cpu,aram,command->mDataLength))throw std::out_of_range("Invalid mapped ARAM transfer");
 lock.unlock();
 doneDMA(reinterpret_cast<std::uintptr_t>(command));
}


extern "C" u32 ARGetBaseAddress(){return 0x4000;}
extern "C" u32 ARInit(u32*,u32){
 std::lock_guard<std::mutex> lock(transportMutex);
 if(!transport || transport->size()<0x4000)throw std::logic_error("Native ARAM backing missing or too small");
 if(!initialized){allocationTop=0x4000;initialized=true;}
 return 0x4000;
}
extern "C" u32 ARGetSize(){std::lock_guard<std::mutex> lock(transportMutex);return initialized && transport?u32(transport->size()):0;}
extern "C" u32 ARAlloc(u32 amount){
 std::lock_guard<std::mutex> lock(transportMutex);
 if(!initialized || !transport)throw std::logic_error("ARAM allocation before initialization");
 if(uint64_t(amount)>transport->size()-allocationTop)throw std::out_of_range("ARAM allocation exceeds registered backing");
 u32 result=u32(allocationTop);allocationTop+=amount;return result;
}
bool nativeUploadAram(uint32_t offset,const void* data,size_t size){
 std::lock_guard<std::mutex> lock(transportMutex);
 if(!transport || !data || !size || size%32 || offset%32)return false;
 auto token=transport->bindCpu(const_cast<void*>(data),size);
 if(!token)return false;
 bool result=transport->transfer(0,token,offset,size);
 transport->unbindCpu(token);
 return result;
}
bool nativeDownloadAram(uint32_t offset,void* data,size_t size){
 std::lock_guard<std::mutex> lock(transportMutex);
 if(!transport || !data || !size || size%32 || offset%32)return false;
 auto token=transport->bindCpu(data,size);
 if(!token)return false;
 bool result=transport->transfer(1,token,offset,size);
 transport->unbindCpu(token);
 return result;
}

JKRAramBlock* JKRAram::mainRamToAram(u8* source,u32 destination,u32 alignedSize,JKRExpandSwitch expandSwitch,u32 fileSize,JKRHeap* heap,int groupId,u32* transferredSize){
 JKRAramBlock* allocatedBlock=nullptr;
 if(!source)return nullptr;
 if(expandSwitch==EXPAND_SWITCH_UNKNOWN1)
  expandSwitch=JKRCheckCompressed_noASR(source)==COMPRESSION_NONE?EXPAND_SWITCH_UNKNOWN0:EXPAND_SWITCH_UNKNOWN1;
 if(expandSwitch==EXPAND_SWITCH_UNKNOWN1){
  const u32 expandedSize=JKRDecompExpandSize(source);
  if(fileSize==0||fileSize>expandedSize)fileSize=(expandedSize+31u)&~31u;
  if(destination==0){
   if(!sAramObject||!sAramObject->mAramHeap)return nullptr;
   allocatedBlock=sAramObject->mAramHeap->alloc(fileSize,JKRAramHeap::HEAD);
   if(!allocatedBlock)return nullptr;
   allocatedBlock->newGroupID(decideAramGroupId(groupId));
   destination=allocatedBlock->getAddress();
  }
  if(alignedSize==0||alignedSize>expandedSize)alignedSize=(expandedSize+31u)&~31u;
  if(alignedSize>fileSize)alignedSize=fileSize;
  void* expanded=JKRAllocFromHeap(heap,fileSize,-32);
  if(!expanded){if(allocatedBlock)delete allocatedBlock;return nullptr;}
  JKRDecompress(source,static_cast<u8*>(expanded),fileSize,0);
  const bool uploaded=nativeUploadAram(destination,expanded,alignedSize);
  JKRFreeToHeap(heap,expanded);
  if(!uploaded){if(allocatedBlock)delete allocatedBlock;return nullptr;}
  if(transferredSize)*transferredSize=alignedSize;
  return allocatedBlock?allocatedBlock:reinterpret_cast<JKRAramBlock*>(static_cast<intptr_t>(-1));
 }
 if(fileSize!=0&&alignedSize>fileSize)alignedSize=fileSize;
 if(destination==0){
  if(!sAramObject||!sAramObject->mAramHeap)return nullptr;
  allocatedBlock=sAramObject->mAramHeap->alloc(alignedSize,JKRAramHeap::HEAD);
  if(!allocatedBlock)return nullptr;
  allocatedBlock->newGroupID(decideAramGroupId(groupId));
  destination=allocatedBlock->getAddress();
 }
 if(!nativeUploadAram(destination,source,alignedSize)){if(allocatedBlock)delete allocatedBlock;return nullptr;}
 if(transferredSize)*transferredSize=alignedSize;
 return allocatedBlock?allocatedBlock:reinterpret_cast<JKRAramBlock*>(static_cast<intptr_t>(-1));
}
