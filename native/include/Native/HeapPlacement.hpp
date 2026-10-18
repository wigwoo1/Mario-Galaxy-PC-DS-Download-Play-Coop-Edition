#pragma once
#include <JSystem/JKernel/JKRHeap.hpp>
#include <cstddef>
#include <limits>
#include <new>

// Preserve the original heap and alignment selection with a host-sized new ABI.
inline void* operator new(std::size_t size, JKRHeap* heap, int alignment) {
    if (size > std::numeric_limits<u32>::max()) throw std::bad_alloc();
    void* memory = JKRHeap::alloc(static_cast<u32>(size), alignment, heap);
    if (!memory) throw std::bad_alloc();
    return memory;
}
inline void operator delete(void* memory, JKRHeap* heap, int) noexcept {
    JKRHeap::free(memory, heap);
}
inline void* operator new[](std::size_t size, JKRHeap* heap, int alignment) {
    return ::operator new(size, heap, alignment);
}
inline void operator delete[](void* memory, JKRHeap* heap, int) noexcept {
    JKRHeap::free(memory, heap);
}
inline void* operator new(std::size_t size, int alignment) {
    return ::operator new(size, static_cast<JKRHeap*>(nullptr), alignment);
}
inline void operator delete(void* memory, int) noexcept {
    JKRHeap::free(memory, nullptr);
}
inline void* operator new[](std::size_t size, int alignment) {
    return ::operator new(size, static_cast<JKRHeap*>(nullptr), alignment);
}
inline void operator delete[](void* memory, int) noexcept {
    JKRHeap::free(memory, nullptr);
}
