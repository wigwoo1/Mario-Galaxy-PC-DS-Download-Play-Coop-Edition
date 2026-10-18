#include <JSystem/JUtility/JUTXfb.hpp>
#include <cassert>
#include <cstdio>
int main(){
 unsigned char buffers[3][64]{};
 auto* x=JUTXfb::createManager(buffers[0],buffers[1],buffers[2]);
 assert(x==JUTXfb::getManager()&&x->getBufferNum()==3);
 assert(!x->getDrawingXfb()&&!x->getDrawnXfb()&&!x->getDisplayingXfb());
 assert(JUTXfb::createManager(nullptr,nullptr,nullptr)==x);
 for(int i=0;i<3;i++){x->setDrawingXfbIndex(i);x->setDrawnXfbIndex(i);x->setDisplayingXfbIndex(i);assert(x->getDrawingXfb()==buffers[i]&&x->getDrawnXfb()==buffers[i]&&x->getDisplayingXfb()==buffers[i]);assert(!x->mXfbAllocated[i]);}
 x->clearIndex();assert(!x->getDrawingXfb()&&!x->getDrawnXfb()&&!x->getDisplayingXfb());
 x->initiate(buffers[0],buffers[1],buffers[2],JUTXfb::UNK_1);assert(x->mXfbs[0]==buffers[0]&&!x->mXfbs[1]&&!x->mXfbs[2]);
 x->initiate(buffers[0],buffers[1],buffers[2],JUTXfb::UNK_2);assert(x->mXfbs[1]==buffers[1]&&!x->mXfbs[2]);
 JUTXfb::destroyManager();assert(!JUTXfb::getManager());buffers[0][0]=42;assert(buffers[0][0]==42);
 JUTXfb::destroyManager();
 std::puts("PASS original XFB borrowed buffer selection, singleton and teardown lifecycle");
}
