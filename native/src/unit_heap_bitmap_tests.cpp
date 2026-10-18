#include <initializer_list>
#include <Native/UnitHeapBitmap.hpp>
#include <cassert>
#include <cstdio>
int main(){uint8_t bytes[5]={0,0x80,1,0x40,2};assert(nativeUnitHeapBitmapWord(bytes+1)==0x80014002u);for(unsigned bit=0;bit<32;++bit){uint8_t map[4]={};map[bit/8]=uint8_t(0x80u>>(bit%8));assert(nativeUnitHeapBitmapWord(map)==(0x80000000u>>bit));}const uintptr_t base=uintptr_t(0x100000000ull)+0x1000;
auto ptr=[](uintptr_t value){return reinterpret_cast<void*>(value);};
assert(nativeUnitHeapAddressIndex(ptr(base),ptr(base),64,256)==0);
assert(nativeUnitHeapAddressIndex(ptr(base+255*64),ptr(base),64,256)==255);
for(uintptr_t invalid:{base-1,base+1,base+256*64,base+0x100000000ull})assert(nativeUnitHeapAddressIndex(ptr(invalid),ptr(base),64,256)==-1);
assert(nativeUnitHeapAddressIndex(nullptr,ptr(base),64,256)==-1);
assert(nativeUnitHeapAddressIndex(ptr(base),ptr(base),0,256)==-1);
std::puts("PASS native unit heap bitmap: all 32 bit positions and unaligned byte reads");}

