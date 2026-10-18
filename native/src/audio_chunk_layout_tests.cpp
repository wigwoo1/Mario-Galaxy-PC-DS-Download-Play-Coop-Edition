#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <cassert>
#include <cstdio>
int main(){using Chunk=JASMemChunkPool<1024,JASThreadingModel::ObjectLevelLockable>::MemoryChunk;Chunk chunk(nullptr);assert(reinterpret_cast<std::uintptr_t>(chunk.mBuffer)%alignof(std::max_align_t)==0);assert(chunk.checkArea(chunk.mBuffer));assert(chunk.checkArea(chunk.mBuffer+1023));assert(!chunk.checkArea(chunk.mBuffer+1024));assert(!chunk.checkArea(&chunk.mUsedSize));auto* first=chunk.alloc(16);auto* second=chunk.alloc(32);assert(first==chunk.mBuffer&&second==chunk.mBuffer+16&&chunk.getFreeSize()==976);chunk.free(first);assert(!chunk.isEmpty());chunk.free(second);assert(chunk.isEmpty());chunk.revive();assert(chunk.getFreeSize()==1024);std::puts("PASS native audio command chunk actual layout, last-byte membership, alignment and reuse");}
