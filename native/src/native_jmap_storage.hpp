#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <map>
// Owns a host-endian image for original JMapInfo. Disk data is never modified.
class NativeJMapStorage {
 std::vector<std::uint8_t> host;
public:
 explicit NativeJMapStorage(const std::vector<std::uint8_t>& disk):host(disk){
  auto extent=[&](std::size_t p,std::size_t n){if(p>disk.size()||n>disk.size()-p)throw std::out_of_range("JMap extent");};
  auto read=[&](std::size_t p,int n){extent(p,n);std::uint32_t v=0;for(int i=0;i<n;++i)v=(v<<8)|disk[p+i];return v;};
  auto put=[&](std::size_t p,int n){auto value=read(p,n);if(n==4)std::memcpy(host.data()+p,&value,4);else if(n==2){auto half=std::uint16_t(value);std::memcpy(host.data()+p,&half,2);}};
  extent(0,16);auto rows=read(0,4),fields=read(4,4),data=read(8,4),stride=read(12,4);
  if(rows>1000000||fields>1024||data<16+fields*12)throw std::invalid_argument("JMap header");extent(16,fields*12);extent(data,std::size_t(rows)*stride);
  for(int p=0;p<16;p+=4)put(p,4);
  std::map<std::uint32_t,int> numericOffsets;
  for(std::uint32_t i=0;i<fields;++i){auto f=16+i*12;auto off=read(f+8,2),type=read(f+11,1),shift=read(f+10,1);int width=type==1?32:type==4?2:type==5?1:4;
   if(type>6||shift>31||off>stride||std::uint32_t(width)>stride-off)throw std::invalid_argument("JMap field");
   put(f,4);put(f+4,4);put(f+8,2);
   if(type!=1){auto found=numericOffsets.find(off);if(found!=numericOffsets.end()&&found->second!=width)throw std::invalid_argument("Conflicting JMap field widths");numericOffsets[off]=width;}
   for(std::uint32_t row=0;row<rows;++row){std::size_t p=data+std::size_t(row)*stride+off;
    if(type==1||type==6){std::size_t start=type==6?data+std::size_t(rows)*stride+read(p,4):p;std::size_t end=type==1?p+32:disk.size();extent(start,1);bool terminated=false;for(auto j=start;j<end;++j)if(!disk[j]){terminated=true;break;}if(!terminated)throw std::invalid_argument("Unterminated JMap string");}
    if(type!=1)put(p,width);
   }
  }
 }
 const void* data()const{return host.data();}
};
