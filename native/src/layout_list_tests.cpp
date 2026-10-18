#include <nw4r/ut/LinkList.h>
#include <cassert>
#include <cstdio>
int main(){nw4r::ut::LinkListNode nodes[32];{
 nw4r::ut::detail::LinkListImpl list;
 for(auto& node:nodes)list.PushBack(&node);
 assert(list.mSize==32);unsigned index=0;for(auto it=list.GetBeginIter();it!=list.GetEndIter();++it)assert(&*it==&nodes[index++]);assert(index==32);
 auto next=list.Erase(&nodes[9]);assert(&*next==&nodes[10] && list.mSize==31);assert(!nodes[9].mNext && !nodes[9].mPrev);
 list.PushFront(&nodes[9]);assert(&*list.GetBeginIter()==&nodes[9] && list.mSize==32);
 auto begin=list.GetBeginIter(),end=begin;for(unsigned i=0;i<4;++i)++end;list.Erase(begin,end);assert(list.mSize==28);assert(!nodes[9].mNext && !nodes[0].mNext && !nodes[1].mNext && !nodes[2].mNext);
 list.Clear();assert(list.mSize==0 && list.GetBeginIter()==list.GetEndIter());for(auto& node:nodes)assert(!node.mNext && !node.mPrev);
 for(auto& node:nodes)list.PushBack(&node);
}
for(auto& node:nodes)assert(!node.mNext && !node.mPrev);std::puts("PASS original NW4R list: insertion, iteration, range erase, reuse and destructor unlink");}
