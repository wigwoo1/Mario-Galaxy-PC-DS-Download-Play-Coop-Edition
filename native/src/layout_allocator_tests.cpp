#include <nw4r/lyt/layout.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
MEMAllocator* nw4r::lyt::Layout::mspAllocator=nullptr;
struct State { unsigned allocs=0,frees=0;void* last=nullptr; };
static void* allocate(MEMAllocator* allocator,u32 size){auto& state=*static_cast<State*>(allocator->pHeap);assert(allocator->heapParam1==32 && allocator->heapParam2==19);++state.allocs;state.last=std::malloc(size);return state.last;}
static void release(MEMAllocator* allocator,void* memory){auto& state=*static_cast<State*>(allocator->pHeap);assert(memory==state.last);++state.frees;std::free(memory);state.last=nullptr;}
int main(){State first,second;const MEMAllocatorFunc callbacks{allocate,release};MEMAllocator a{&callbacks,&first,32,19},b{&callbacks,&second,32,19};
for(unsigned i=1;i<=256;++i){nw4r::lyt::Layout::mspAllocator=i%2?&a:&b;auto memory=static_cast<unsigned char*>(nw4r::lyt::Layout::AllocMemory(i));assert(memory);for(unsigned j=0;j<i;++j)memory[j]=static_cast<unsigned char>(j);nw4r::lyt::Layout::FreeMemory(memory);}
assert(first.allocs==128 && first.frees==128 && second.allocs==128 && second.frees==128);nw4r::lyt::Layout::mspAllocator=nullptr;std::puts("PASS original UI allocator dispatch: 256 allocations, two native callback contexts");}
