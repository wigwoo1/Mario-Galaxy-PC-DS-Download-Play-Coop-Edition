#include <JSystem/JKernel/JKRAramHeap.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cassert>
#include <cstdio>
#include <initializer_list>
int main(){alignas(32) unsigned char storage[65536]{};JKRExpHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;parent.becomeCurrentHeap();const auto initial=parent.getTotalFreeSize();{JKRAramHeap heap(0,1024);for(auto mode:{JKRAramHeap::HEAD,JKRAramHeap::TAIL}){assert(!heap.alloc(0,mode));assert(!heap.alloc(0xffffffffu,mode));assert(!heap.alloc(0xffffffe1u,mode));}auto head=heap.alloc(33,JKRAramHeap::HEAD);auto tail=heap.alloc(64,JKRAramHeap::TAIL);assert(head && tail && head->getAddress()==0 && head->getSize()==64 && tail->getAddress()==960 && tail->getSize()==64);assert(!heap.alloc(1024,JKRAramHeap::HEAD));delete head;delete tail;auto all=heap.alloc(1024,JKRAramHeap::HEAD);assert(all && all->getAddress()==0 && all->getSize()==1024);delete all;}assert(JKRAramHeap::sAramList.getNumLinks()==0);assert(parent.check());assert(parent.getTotalFreeSize()==initial);std::puts("PASS original ARAM heap head/tail allocation, alignment, coalescing and teardown");}


