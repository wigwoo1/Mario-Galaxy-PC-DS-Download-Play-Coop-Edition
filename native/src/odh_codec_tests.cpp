#include "Game/Screen/odh.hpp"
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdio>
int main(){
 for(unsigned side:{8u,16u,32u}){
  std::vector<u8> src(side*side*2+32,0),work(side*side*3+32,0xaa),output(4096+32,0xcc);
  for(unsigned i=0;i<side*side;i++){src[i*2]=u8((i%3)*83);src[i*2+1]=u8(i*31);}
  auto length=ODHEncodeRGB565(src.data(),output.data(),side,side,4096,80,work.data());
  assert(length>16&&length<=4096&&!std::memcmp(output.data(),"AJPG",4));
  auto readBE=[&](int o){return unsigned(output[o])<<24|unsigned(output[o+1])<<16|unsigned(output[o+2])<<8|output[o+3];};
  unsigned packed=readBE(4);assert((packed>>24)<=80&&(packed>>24)>0&&(packed&2047)==side&&((packed>>11)&2047)==side);
  assert(readBE(8)==length&&readBE(12)==0);
  for(unsigned i=4096;i<output.size();i++)assert(output[i]==0xcc);
  for(unsigned i=side*side*3;i<work.size();i++)assert(work[i]==0xaa);
 }
 std::vector<std::vector<u8>> colorPayloads;
 for(unsigned color:{0u,0xf800u,0x001fu,0xffffu}){
  constexpr unsigned width=488,height=328,capacity=40960;
  std::vector<u8> source(width*height*2,0),work(width*height*3+32,0xaa),out(capacity+32,0xcc);
  for(unsigned i=0;i<width*height;i++){source[i*2]=u8(color>>8);source[i*2+1]=u8(color);}
  unsigned size=ODHEncodeRGB565(source.data(),out.data(),width,height,capacity,80,work.data());
  assert(size>16&&size<=capacity);
  for(unsigned i=capacity;i<out.size();i++)assert(out[i]==0xcc);
  for(unsigned i=width*height*3;i<work.size();i++)assert(work[i]==0xaa);
  colorPayloads.emplace_back(out.begin()+16,out.begin()+size);
 }
 for(unsigned i=0;i<colorPayloads.size();i++)for(unsigned j=i+1;j<colorPayloads.size();j++)assert(colorPayloads[i]!=colorPayloads[j]);
 std::puts("PASS actual 488x328 capture size/40KB capacity guards and distinct black/red/blue/white compressed payloads");
 std::puts("PASS native original AJPG encode: tiled RGB565 fixtures, byte-order header, output/work guards; decoder fidelity unverified");
}

