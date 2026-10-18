#include <stdint.h>
extern "C" void OSPanic(const char*,int,const char*,...);
void JKRDefaultMemoryErrorRoutine(void* heap,uint32_t size,int alignment){
 OSPanic("JKRHeap",0x355,"Heap allocation failed: heap=%p size=%u alignment=%d",heap,size,alignment);
}
