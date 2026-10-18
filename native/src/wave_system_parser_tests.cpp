#include <JSystem/JAudio2/JASWSParser.hpp>
#include <cassert>
#include <cstdio>
int main(){alignas(4) u8 b[128]{};b[0x17]=32;b[40]=0;b[43]=2;auto* h=reinterpret_cast<JASWSParser::THeader*>(b);auto* group=h->mCtrlGroupOffset.ptr(h);assert(reinterpret_cast<u8*>(group)==b+32&&nativeWSU32(&group->mGroupCount)==2);u8 integer[]={0x12,0x34,0x56,0x78};assert(nativeWSU32(integer)==0x12345678);u8 signedValue[]={0xff,0x85};assert(nativeWSS16(signedValue)==-123);u8 rate[]={0x46,0xfa,0,0};assert(nativeWSFloat(rate)==32000);u8 unaligned[]={0,0x3f,0x80,0,0};assert(nativeWSFloat(unaligned+1)==1);puts("PASS wave-system native big-endian offsets, counts, signed samples and floating rates.");}
