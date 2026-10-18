#include <revolution/mem/allocator.h>
void* MEMAllocFromAllocator(MEMAllocator *pAllocator, u32 size) {
    return (*pAllocator->pFunc->pfAlloc)(pAllocator, size);
}

void MEMFreeToAllocator(MEMAllocator *pAllocator, void *pBlock) {
    (*pAllocator->pFunc->pfFree)(pAllocator, pBlock);
}

