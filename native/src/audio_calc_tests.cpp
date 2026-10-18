#include <JSystem/JAudio2/JASCalc.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cmath>
extern "C" void DCZeroRange(void* memory,u32 size){std::memset(memory,0,size);}
int main(){alignas(32) unsigned char source[128],destination[128];for(unsigned i=0;i<128;++i)source[i]=i;for(unsigned offset=0;offset<8;++offset)for(unsigned count=0;count<=64;++count){std::memset(destination,0xa5,128);JASCalc::bcopy(source+offset,destination+offset,count);assert(!std::memcmp(source+offset,destination+offset,count));assert(destination[offset+count]==0xa5);std::memset(destination,0xa5,128);JASCalc::bzero(destination+offset,count);for(unsigned i=0;i<count;++i)assert(destination[offset+i]==0);assert(destination[offset+count]==0xa5);}short a[]={1,2,3},b[]={4,5,6},mixed[6];JASCalc::imixcopy(a,b,mixed,3);for(unsigned i=0;i<3;++i){assert(mixed[2*i]==a[i]&&mixed[2*i+1]==b[i]);}assert(std::fabs(JASCalc::pow2(0)-1)<.001f&&std::fabs(JASCalc::pow2(1)-2)<.001f&&std::fabs(JASCalc::pow2(-1)-.5f)<.001f);std::puts("PASS original audio byte copy/zero ranges, stereo interleave and integer pitch powers (cache-zero fixture)");}

