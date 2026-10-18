#include <Game/RhythmLib/AudChordInfo.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){u8 data[64]{};std::memcpy(data+4,"CITS",4);data[13]=1;data[15]=1;data[19]=24;data[23]=32;for(int i=0;i<8;++i)data[24+i]=u8(i+1);data[35]=40;data[39]=52;for(int i=0;i<12;++i){data[40+i]=u8(i);data[52+i]=u8(11-i);}u8 before[64];std::memcpy(before,data,64);AudChordTable table;assert(table.setChordTableResource(data));assert(table.mChordCount==1&&table.mScaleCount==1&&table.mChordPtr[0]->mBass==1&&table.mChordPtr[0]->mAddNote[2]==8);for(int i=0;i<12;++i)assert(table.mScalePtr[0]->up[i]==i&&table.mScalePtr[0]->down[i]==11-i);assert(!std::memcmp(before,data,64));assert(table.setChordTableResource(data));assert(!table.setChordTableResource(nullptr));data[4]='X';assert(!table.setChordTableResource(data));std::puts("PASS native chord table: Wii BE counts/offsets, chord/scale lookup, 64-bit pointers, immutable resource and reload");}
