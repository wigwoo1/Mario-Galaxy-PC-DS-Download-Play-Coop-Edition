#include <vector>
#include <iterator>
#include <JSystem/JGadget/linklist.hpp>
#include <JSystem/JAudio2/JASGadget.hpp>
#include <cstddef>
#include <cstdio>
struct Entry { u64 id; JGadget::TLinkListNode node; };
using Intrusive=JGadget::TLinkList<Entry,-static_cast<int>(offsetof(Entry,node))>;
int main(){
    std::vector<int> standard{1,2,3};if(std::distance(standard.begin(),standard.end())!=3)return 1;
    Entry first{11,{}},second{22,{}};first.node.mNext=&second.node;first.node.mPrev=&second.node;second.node.mNext=&first.node;second.node.mPrev=&first.node;
    if(Intrusive::Element_toNode(&first)!=&first.node||Intrusive::Element_toValue(&first.node)!=&first)return 2;
    Intrusive::iterator it(&first.node);if(it->id!=11)return 3;++it;if(it->id!=22)return 4;--it;if(it->id!=11)return 5;auto old=it--;if(old->id!=11||it->id!=22)return 6;it=old;if(it->id!=11)return 7;
    JASPtrArray<int,4> table;for(unsigned i=0;i<4;++i)if(table.get(i)!=nullptr)return 8;int payload=99;table.set(3,&payload);if(table.get(3)!=&payload||*table.get(3)!=99||table.get(4)!=nullptr)return 9;if(table.mPtrTable!=table.mPtrArray||table.mLen!=4)return 10;
    std::puts("PASS actual intrusive audio iterator conversion/traversal and native pointer-table initialization/lookup");
}
