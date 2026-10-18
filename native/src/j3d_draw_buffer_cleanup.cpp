#include <JSystem/J3DGraphBase/J3DDrawBuffer.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
J3DDrawBuffer::~J3DDrawBuffer(){if(mpBuffer)JKRHeap::free(mpBuffer,JKRHeap::findFromRoot(mpBuffer));}
