#include <JSystem/JKernel/JKRHeap.hpp>
#include <array>
#include <cassert>
#include <cstdio>
int main(){for(unsigned size=0;size<=128;++size){alignas(32) std::array<unsigned char,160> source,dest;for(unsigned i=0;i<source.size();++i)source[i]=i*17+3;dest.fill(0xaa);JKRHeap::copyMemory(dest.data(),source.data(),size);unsigned rounded=(size+3)&~3u;for(unsigned i=0;i<dest.size();++i)assert(dest[i]==(i<rounded?source[i]:0xaa));}std::puts("PASS original heap copy: 129 sizes with original word rounding and bounds");}
