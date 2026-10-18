#include <JSystem/JUtility/JUTDirectPrint.hpp>
#include <Native/GpuResources.hpp>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <cstdio>
int main(){
 JUTDirectPrint printer;
 assert(!printer.mFrameBuffer && printer.mFrameBufferSize==0);
 printer.drawString_f(0,0,"ignored");
 constexpr unsigned w=3200,h=32,guard=32;
 std::vector<u16> storage(w*h+guard*2,0x7654);
 u16* pixels=storage.data()+guard;
 printer.changeFrameBuffer(pixels,w,h);
 assert(printer.mStride==w&&printer.mFrameBufferSize==w*h*2);
 auto& gpu=Native::gpuResources();auto address=gpu.add(pixels,w*h*2);

 std::string longText(1000,'A');printer.drawString_f(0,0,"%s",longText.c_str());

 assert(std::memcmp(gpu.resolve(address,w*h*2),pixels,w*h*2)==0);
 for(unsigned i=0;i<guard;i++){assert(storage[i]==0x7654&&storage[guard+w*h+i]==0x7654);}
 // Exactly 255 stored glyphs, each 12 pixels in doubled-width mode.
 bool painted=false;for(unsigned y=0;y<14;y++)for(unsigned x=0;x<3060;x++)painted|=pixels[y*w+x]!=0x7654;
 assert(painted);
 for(unsigned y=0;y<h;y++)for(unsigned x=3060;x<w;x++)assert(pixels[y*w+x]==0x7654);
 printer.changeFrameBuffer(nullptr,0,0);printer.drawString_f(0,0,"ignored");
 gpu.remove(pixels);
 std::puts("PASS original direct-print truncation, glyph extent, guard storage and native cache snapshot refresh");
}

