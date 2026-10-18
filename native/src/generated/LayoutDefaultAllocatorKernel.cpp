#include <Game/Util/MemoryUtil.hpp>
namespace MR {
    void* NewDeleteAllocator::alloc(MEMAllocator* pAllocator, u32 size) {
        return JKRHeap::alloc(size, alignof(std::max_align_t), nullptr);
    }

    void NewDeleteAllocator::free(MEMAllocator* pAllocator, void* pPtr) {
        JKRHeap::free(pPtr, nullptr);
    }

    MEMAllocatorFunc NewDeleteAllocator::sAllocatorFunc = {
        NewDeleteAllocator::alloc,
        NewDeleteAllocator::free,
    };
    MEMAllocator NewDeleteAllocator::sAllocator = {&sAllocatorFunc};


}
