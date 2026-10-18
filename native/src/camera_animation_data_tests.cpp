#include <Native/CameraAnimationData.hpp>
#include <cassert>
#include <cstdio>
static void put(std::vector<uint8_t>& b,size_t p,uint32_t v){for(int i=3;i>=0;--i){b[p+i]=v&255;v>>=8;}}
int main(){
 std::vector<uint8_t> b(100);std::memcpy(b.data(),"ANDOCANM",8);
 put(b,12,0xfffffffd);put(b,16,7);put(b,20,0x12345678); put(b,8,1);put(b,24,120);put(b,28,64);
 for(int i=0;i<8;++i){put(b,32+i*8,1);put(b,36+i*8,0);}
 put(b,96,4);b.resize(104);put(b,100,0x42280000);
 Native::CameraAnimationData a;assert(a.decode(b.data(),b.size()));assert(a.frames==120&&a.values[0]==42);
 auto host=a.hostFile();uint32_t h=0;std::memcpy(&h,host.data()+12,4);assert(h==0xfffffffd);
 std::memcpy(&h,host.data()+16,4);assert(h==7);
 std::memcpy(&h,host.data()+20,4);assert(h==0x12345678);
 float value=0;std::memcpy(&value,host.data()+100,4);assert(value==42);
 // Neither decoder storage nor host storage aliases the archive input.
 b[100]=0;assert(a.values[0]==42);std::memcpy(&value,host.data()+100,4);assert(value==42);put(b,100,0x42280000);
 for(size_t n=0;n<b.size();++n)assert(!a.decode(b.data(),n));
 assert(a.frames==120&&a.values[0]==42);
 put(b,36,1);assert(!a.decode(b.data(),b.size()));put(b,36,0);
 put(b,100,0x7f800000);assert(!a.decode(b.data(),b.size()));
 assert(a.values[0]==42);assert(!a.decode(nullptr,104));
 std::vector<uint8_t> k(156);std::memcpy(k.data(),"ANDOCKAN",8);
 std::memcpy(k.data()+4,"CKAN",4);put(k,8,1);put(k,24,30);put(k,28,96);put(k,128,24);
 for(int i=0;i<8;++i){put(k,32+i*12,2);put(k,36+i*12,0);put(k,40+i*12,0);}
 put(k,132,0);put(k,136,0x42280000);put(k,140,0);put(k,144,0x41f00000);put(k,148,0x42700000);put(k,152,0);
 assert(a.decode(k.data(),k.size()));assert(a.keyed&&a.values[4]==60);
 host=a.hostFile();std::memcpy(&h,host.data()+28,4);assert(h==96);
 std::memcpy(&h,host.data()+32,4);assert(h==2);
 std::memcpy(&value,host.data()+148,4);assert(value==60);
 put(k,144,0);assert(!a.decode(k.data(),k.size()));assert(a.values[4]==60);
 put(k,144,0x41f00000);put(k,40,2);assert(!a.decode(k.data(),k.size()));
 std::puts("PASS camera animation endian conversion, all truncations, component bounds and transactional rejection");
}


