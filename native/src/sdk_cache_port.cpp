#include <Native/GpuResources.hpp>
#include <revolution/os/OSCache.h>
#include <revolution/base/PPCArch.h>
#include <atomic>

extern "C" {
void PPCSync() { std::atomic_thread_fence(std::memory_order_seq_cst); }
void DCStoreRangeNoSync(void* data,u32 size) { Native::gpuResources().refresh(data,size); }
void DCFlushRangeNoSync(void* data,u32 size) { Native::gpuResources().refresh(data,size); }
void DCStoreRange(void* data,u32 size) { DCStoreRangeNoSync(data,size); PPCSync(); }
void DCFlushRange(void* data,u32 size) { DCFlushRangeNoSync(data,size); PPCSync(); }
}

// Native CPU memory is coherent; order externally completed writes before reads.
extern "C" void DCInvalidateRange(void*,u32){std::atomic_thread_fence(std::memory_order_seq_cst);}
