#include "nw4r/ut/LinkList.h"
#include "revolution/mem/heapCommon.h"
#include <cstddef>
struct Item { u64 payload; nw4r::ut::LinkListNode node; };
using List=nw4r::ut::LinkList<Item,offsetof(Item,node)>;
int main(){
    static_assert(sizeof(nw4r::IntPtr)==sizeof(void*)&&sizeof(UIntPtr)==sizeof(void*));
    Item item;auto* node=List::GetNodeFromPointer(&item);
    if(node!=&item.node||List::GetPointerFromNode(node)!=&item)return 1;
    const auto* constNode=node;if(List::GetPointerFromNode(constNode)!=&item)return 2;
    char bytes[32];if(AddU32ToPtr(bytes,17)!=bytes+17||SubU32ToPtr(bytes+17,17)!=bytes)return 3;
    if(GetOffsetFromPtr(bytes,bytes+17)!=17||GetUIntPtr(bytes)!=reinterpret_cast<std::uintptr_t>(bytes))return 4;
    return 0;
}
