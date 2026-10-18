#include <array>
#include <cassert>
#include <cstdio>
#include <Game/Util/MemoryUtil.hpp>
int main(){for(unsigned source=0;source<4;++source)for(unsigned dest=0;dest<32;++dest)for(unsigned size=0;size<=128;++size){alignas(32) std::array<unsigned char,192> a,b;for(unsigned i=0;i<a.size();++i)a[i]=unsigned(i*17+3);b.fill(0xaa);MR::copyMemory(b.data()+dest,a.data()+source,size);for(unsigned i=0;i<b.size();++i)assert(b[i]==(i>=dest && i<dest+size?a[source+i-dest]:0xaa));b.fill(0xaa);MR::zeroMemory(b.data()+dest,size);for(unsigned i=0;i<b.size();++i)assert(b[i]==(i>=dest && i<dest+size?0:0xaa));b.fill(0xaa);MR::fillMemory(b.data()+dest,0x5b,size);for(unsigned i=0;i<b.size();++i)assert(b[i]==(i>=dest && i<dest+size?0x5b:0xaa));}std::puts("PASS original native copy/zero/fill: 16512 offset/size combinations with boundary guards");}
