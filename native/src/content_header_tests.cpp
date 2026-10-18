#include <Game/System/BinaryDataContentAccessor.hpp>
#include <Game/Util/HashUtil.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
int main(){
 u8 header[32]{};BinaryDataContentHeaderSerializer writer(header,32);
 writer.addAttribute("mGalaxyName",2);writer.addAttribute("mPowerStarFlag",1);
 writer.addAttribute("mFirstPlayFlag",1);writer.addAttribute("mMaxCoinNum",16);writer.flush();
 assert(writer.getHeaderSize()==20&&writer.getDataSize()==20);
 assert(header[0]==0&&header[1]==4&&header[2]==0&&header[3]==20);
 const char* names[]={"mGalaxyName","mPowerStarFlag","mFirstPlayFlag","mMaxCoinNum"};
 const u16 offsets[]={0,2,3,4};
 for(int i=0;i<4;++i){
  const u16 hash=u16(MR::getHashCode(names[i]));const auto p=header+4+i*4;
  assert(p[0]==u8(hash>>8)&&p[1]==u8(hash)&&p[2]==0&&p[3]==offsets[i]);
 }
 BinaryDataContentAccessor reader(header);u8 record[20]{};
 assert(reader.getHeaderSize()==20&&reader.getDataSize()==20&&reader.getAttributeNum()==4);
 for(int i=0;i<4;++i)assert(reader.getPointer(names[i],record)==record+offsets[i]);
 assert(!reader.getPointer("missing",record));
 auto old=header[19];header[19]=30;assert(!reader.getPointer("mMaxCoinNum",record));header[19]=old;
 u8 small[8]{};BinaryDataContentHeaderSerializer bounded(small,8);bounded.addAttribute("first",2);bounded.flush();
 u8 snapshot[8];std::memcpy(snapshot,small,8);bool rejected=false;
 try{bounded.addAttribute("second",1);}catch(const std::length_error&){rejected=true;}
 assert(rejected&&std::memcmp(snapshot,small,8)==0&&bounded.getDataSize()==2);
 std::puts("PASS original save content header: big-endian hashes/offsets, exact GALA layout, lookup and capacity failure preservation");
}




