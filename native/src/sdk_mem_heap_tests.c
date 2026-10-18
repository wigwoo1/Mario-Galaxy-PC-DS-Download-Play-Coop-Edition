#include <windows.h>
#include <revolution/mem/expHeap.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void OSInitMutex(OSMutex* mutex){memset(mutex,0,sizeof(*mutex));}
// Single-thread allocator regression; runtime supplies the real OS mutex port.
void OSLockMutex(OSMutex* mutex){(void)mutex;}
void OSUnlockMutex(OSMutex* mutex){(void)mutex;}
s32 __abs(s32 x){return x<0?-x:x;}

int main(){
 const size_t bytes=2*1024*1024;
 unsigned char* base=(unsigned char*)(VirtualAlloc((void*)((uintptr_t)1<<40),bytes,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
 assert(base&&(uintptr_t)base>UINT32_MAX);
 if(GetUIntPtr(base)!=(uintptr_t)base||RoundUpPtr(base+1,32)!=base+32){puts("FAIL SDK heap pointer helpers truncate a real address above 4GiB");VirtualFree(base,0,MEM_RELEASE);return 21;}
 assert(ComparePtr(base,base+1)<0&&ComparePtr(base+1,base)>0&&ComparePtr(base,base)==0);
 MEMHeapHandle heap=MEMCreateExpHeapEx(base+1,bytes-2,1);
 assert(heap&&(uintptr_t)heap%__alignof__(MEMiHeapHead)==0);
 const u32 available=MEMGetAllocatableSizeForExpHeapEx(heap,4);assert(available>bytes-4096);
 void* blocks[72];size_t blockCount=0;
 const int directions[]={1,-1}, alignments[]={4,8,16,32,64,256};const unsigned sizes[]={0,1,3,9,65,257}; for(int d=0;d<2;++d)for(int a=0;a<6;++a)for(int z=0;z<6;++z){int direction=directions[d],alignment=alignments[a];unsigned size=sizes[z];
  void* block=MEMAllocFromExpHeapEx(heap,size,direction*alignment);assert(block);
  assert((uintptr_t)block%alignment==0&&(uintptr_t)block%__alignof__(MEMiExpHeapMBlockHead)==0);
  assert(((unsigned char*)block)>=base&&((unsigned char*)block)+(size?size:1)<=base+bytes);
  assert(*((unsigned char*)block)==0);memset(block,0xAB,size?size:1);blocks[blockCount++]=block;
 }
 for(size_t i=0;i<blockCount;i+=2)MEMFreeToExpHeap(heap,blocks[i]);
 for(size_t i=1;i<blockCount;i+=2)MEMFreeToExpHeap(heap,blocks[i]);
 assert(MEMGetAllocatableSizeForExpHeapEx(heap,4)==available);
 // Two root heaps and nested children exercise real MEM list links above4GiB.
 void* childBuffer=MEMAllocFromExpHeapEx(heap,65536,32);assert(childBuffer);
 MEMHeapHandle child=MEMCreateExpHeapEx(childBuffer,65536,1);assert(child);
 void* nested=MEMAllocFromExpHeapEx(child,8192,-32);assert(nested);
 MEMHeapHandle grandchild=MEMCreateExpHeapEx(nested,8192,1);assert(grandchild);
 assert(MEMAllocFromExpHeapEx(grandchild,33,4));MEMDestroyExpHeap(grandchild);
 MEMFreeToExpHeap(child,nested);MEMDestroyExpHeap(child);MEMFreeToExpHeap(heap,childBuffer);
 unsigned char* second=(unsigned char*)(VirtualAlloc(NULL,65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));assert(second);
 MEMHeapHandle sibling=MEMCreateExpHeapEx(second,65536,0);assert(sibling);MEMDestroyExpHeap(sibling);VirtualFree(second,0,MEM_RELEASE);
 assert(!MEMAllocFromExpHeapEx(heap,UINT32_MAX,4));assert(!MEMAllocFromExpHeapEx(heap,UINT32_MAX-15,256));assert(!MEMAllocFromExpHeapEx(heap,UINT32_MAX-15,-256));assert(!MEMAllocFromExpHeapEx(heap,8,0));assert(!MEMAllocFromExpHeapEx(heap,8,3));
 assert(MEMGetAllocatableSizeForExpHeapEx(heap,4)==available);
 void* whole=MEMAllocFromExpHeapEx(heap,available,4);assert(whole&&!MEMAllocFromExpHeapEx(heap,8,4));MEMFreeToExpHeap(heap,whole);
 MEMFreeToExpHeap(heap,NULL);MEMDestroyExpHeap(heap);assert(!MEMCreateExpHeapEx(base,8,0));VirtualFree(base,0,MEM_RELEASE);
 puts("PASS original SDK heaps above4GiB: full pointers, host header alignment, head/tail allocation, zero fill, split/coalesce, nested/root lists, exhaustion and reuse");
}
