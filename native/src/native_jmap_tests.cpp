#include "native_jmap_storage.hpp"
#include <Game/Util/JMapInfo.hpp>
#include <Game/Map/CollisionCode.hpp>
#include <fstream>
#include <iterator>
#include <cassert>
#include <cstdio>
int main(int argc,char** argv){assert(argc==2);std::ifstream in(argv[1],std::ios::binary);assert(in);std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),{});NativeJMapStorage native(bytes);JMapInfo info;assert(info.attachNative(bytes));JMapInfo copied(info);{auto temporary=bytes;assert(info.attachNative(temporary));}assert(copied.getNumEntries()==info.getNumEntries());std::size_t cells=0;
 auto read=[&](std::size_t p,int n){std::uint32_t v=0;for(int i=0;i<n;++i)v=(v<<8)|bytes.at(p+i);return v;};
 for(int row=0;row<info.getNumEntries();++row)for(int col=0;col<info.getNumFields();++col){auto f=16+col*12;auto type=read(f+11,1),off=read(f+8,2);auto p=read(8,4)+row*read(12,4)+off;
  if(type==0||type==3||type==4||type==5){u32 value;assert(info.getValueFast(row,col,&value));auto expected=(read(p,type==4?2:type==5?1:4)&read(f+4,4))>>read(f+10,1);assert(value==expected);}
  else if(type==2){float value;info.getValueFast(row,col,&value);auto bits=read(p,4);float expected;std::memcpy(&expected,&bits,4);assert(std::memcmp(&value,&expected,4)==0);}
  else {const char* value;info.getValueFast(row,col,&value);std::size_t start=type==6?read(8,4)+read(0,4)*read(12,4)+read(p,4):p;assert(std::strcmp(value,reinterpret_cast<const char*>(bytes.data()+start))==0);}
  u32 copiedValue;if(type==0||type==3||type==4||type==5){u32 liveValue;assert(copied.getValueFast(row,col,&copiedValue));assert(info.getValueFast(row,col,&liveValue));assert(liveValue==copiedValue);}
  ++cells;
 }std::size_t codeChecks=0;CollisionCode codes;for(int row=0;row<info.getNumEntries();++row){JMapInfoIter iter(&info,row);const char* names[] = {"Floor_code","Wall_code","Sound_code","Camera_through"};u32 results[]={codes.getFloorCode(iter),codes.getWallCode(iter),codes.getSoundCode(iter),codes.getCameraCode(iter)};
 for(int k=0;k<4;++k){if(info.getValueType(names[k])==0){u32 expected=0;assert(info.getValue(row,names[k],&expected));assert(results[k]==expected);++codeChecks;}}
 u32 camera=~0u;info.getValue(row,"camera_id",&camera);assert(codes.getCameraID(iter)==camera);
}
if(cells>3)assert(codeChecks>0);std::printf("PASS original CollisionCode: %zu numeric attribute selections\n",codeChecks);
for(auto table:{codes.mFloorTable,codes.mWallTable,codes.mSoundTable,codes.mCameraTable}){delete[] table->mHashTable;delete[] table->mCodeTable;delete[] table->mNameTable;delete table;}
std::printf("PASS original JMapInfo: %zu decoded game-table cells\n",cells);
}
