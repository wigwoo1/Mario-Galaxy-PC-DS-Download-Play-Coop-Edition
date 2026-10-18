#include "Native/HeapPlacement.hpp"
#include <Game/Util/Functor.hpp>
#include <memory>
#include <cassert>
#include <cstdlib>
#include <cstdio>
static JKRHeap* expectedHeap;
static unsigned calls, frees;
static int expectedAlignment;
static bool failAllocation;
void* JKRHeap::alloc(u32 size, int alignment, JKRHeap* heap) {
    assert(heap == expectedHeap && alignment == expectedAlignment);
    ++calls;
    return failAllocation ? nullptr : std::malloc(size);
}
void JKRHeap::free(void* memory, JKRHeap* heap) {
    assert(heap == expectedHeap);
    ++frees;
    std::free(memory);
}
struct Throws { Throws() { throw 7; } };
static unsigned callbackCalls;
static void shadowCallback() { ++callbackCalls; }
int main() {
    expectedHeap = nullptr; expectedAlignment = 32;
    void* memory = ::operator new(48, expectedHeap, expectedAlignment);
    ::operator delete(memory, expectedHeap, expectedAlignment);
    assert(calls == 1 && frees == 1);
    try { new (expectedHeap, expectedAlignment) Throws; assert(false); }
    catch (int value) { assert(value == 7); }
    assert(calls == 2 && frees == 2);
    failAllocation = true;
    try { ::operator new(48, expectedHeap, expectedAlignment); assert(false); }
    catch (const std::bad_alloc&) {}
    const auto before = calls;
    try { ::operator new(static_cast<std::size_t>(std::numeric_limits<u32>::max()) + 1, expectedHeap, expectedAlignment); assert(false); }
    catch (const std::bad_alloc&) {}
    assert(calls == before);
    failAllocation = false;
    memory = ::operator new[](96, expectedHeap, expectedAlignment);
    ::operator delete[](memory, expectedHeap, expectedAlignment);
    memory = ::operator new[](96, expectedAlignment);
    ::operator delete[](memory, expectedAlignment);
    const auto beforeFree = frees;
    try { new (expectedAlignment) Throws[3]; assert(false); }
    catch (int value) { assert(value == 7); }
    assert(frees == beforeFree + 1);
    // Opaque allocator identity is compared only; the test never dereferences it.
    expectedHeap = reinterpret_cast<JKRHeap*>(std::uintptr_t(0x100000001ull));
    expectedAlignment = alignof(MR::NativeCallbackAllocation);
    auto callback = MR::Functor(&shadowCallback);
    const auto cloneCalls = calls, cloneFrees = frees;
    {
        std::unique_ptr<MR::FunctorBase> first(callback.clone(expectedHeap));
        std::unique_ptr<MR::FunctorBase> second(first->clone(expectedHeap));
        std::unique_ptr<MR::FunctorBase> ordinary(first->clone(nullptr));
        callback(); (*first)(); (*second)(); (*ordinary)();
        assert(callbackCalls == 4 && calls == cloneCalls + 2);
    }
    assert(frees == cloneFrees + 2);
    failAllocation = true;
    try { std::unique_ptr<MR::FunctorBase> rejected(callback.clone(expectedHeap)); assert(false); }
    catch (const std::bad_alloc&) {}
    assert(frees == cloneFrees + 2);
    std::puts("PASS free-function callback dispatch, clone chains, heap identity, cleanup and allocation failure (allocator double)");
    std::puts("PASS host heap placement forwarding, failure, overflow and constructor cleanup (allocator double)");
}
