#pragma once
#include <atomic>
struct NativeThreadCancelled {};
inline thread_local std::atomic<bool>* nativeThreadCancellation=nullptr;
inline thread_local void (*nativeThreadSuspensionPoint)()=nullptr;
inline void nativeThreadCancellationPoint(){if(nativeThreadCancellation && nativeThreadCancellation->load(std::memory_order_acquire))throw NativeThreadCancelled{};if(nativeThreadSuspensionPoint)nativeThreadSuspensionPoint();}

