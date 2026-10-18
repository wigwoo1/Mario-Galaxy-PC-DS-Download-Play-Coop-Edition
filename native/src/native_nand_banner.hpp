#pragma once
#include <revolution/nand.h>
#include <cstring>
namespace Native {
inline void encodeNandBanner(NANDBanner* banner){
 auto bytes=reinterpret_cast<u8*>(banner);u32 signature=banner->signature,flag=banner->flag;u16 speed=banner->iconSpeed;
 bytes[0]=signature>>24;bytes[1]=signature>>16;bytes[2]=signature>>8;bytes[3]=signature;
 bytes[4]=flag>>24;bytes[5]=flag>>16;bytes[6]=flag>>8;bytes[7]=flag;bytes[8]=speed>>8;bytes[9]=speed;
 for(int row=0;row<2;++row)for(int i=0;i<32;++i){u16 value=banner->comment[row][i];u8* p=bytes+32+row*64+i*2;p[0]=value>>8;p[1]=value;}
}
}
