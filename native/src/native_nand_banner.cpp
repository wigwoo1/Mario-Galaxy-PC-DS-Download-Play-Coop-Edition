#include "native_nand_banner.hpp"
extern "C" void NANDInitBanner(NANDBanner* banner,u32 flag,const u16* title,const u16* comment){
 std::memset(banner,0,sizeof(*banner));banner->signature=0x5749424e;banner->flag=flag;
 const u16* lines[]={title,comment};for(int row=0;row<2;++row){const u16* line=lines[row];if(!line||!*line){banner->comment[row][0]=' ';continue;}for(int i=0;i<32&&line[i];++i)banner->comment[row][i]=line[i];}
}
