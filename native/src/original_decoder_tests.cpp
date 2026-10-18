#include <JSystem/JKernel/JKRDecomp.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){
 u8 yaz[22]={'Y','a','z','0',0,0,0,6,0,0,0,0,0,0,0,0,0xE0,'A','B','C',0x10,2};
 u8 out[8]={}; JKRDecomp::decodeSZS(yaz,out,6,0);
 assert(std::memcmp(out,"ABCABC",6)==0 && out[6]==0);
 u8 yay[23]={'Y','a','y','0',0,0,0,6,0,0,0,20,0,0,0,22,0xC0,0,0,0,0x20,1,'X'};
 // Two literals need two source bytes; use an explicit larger valid stream.
 u8 stream[24]; std::memcpy(stream,yay,23); stream[23]='Y';
 std::memset(out,0,8); JKRDecomp::decodeSZP(stream,out,6,0);
 assert(std::memcmp(out,"XYXYXY",6)==0 && out[6]==0);
 assert(JKRDecomp::checkCompressed(yaz)==JKR_COMPRESSION_SZS);
 assert(JKRDecomp::checkCompressed(stream)==JKR_COMPRESSION_SZP);
 std::puts("Original adapted decoder PASS: big-endian sizes, literals and overlapping backreferences in Yaz0/Yay0");
}
