#include <Native/TevArithmetic.hpp>
#include <cassert>
#include <cstdio>
int main(){
 using Native::tevArithmetic;
 // Independently specified endpoint, rounding-tie and saturation results.
 assert(tevArithmetic(0,128,128,200,0,false,0,false)==265);
 assert(tevArithmetic(0,128,128,200,0,true,0,false)==136);
 assert(tevArithmetic(0,128,128,200,0,false,0,true)==255);
 assert(tevArithmetic(0,255,255,0,0,false,3,false)==127);
 assert(tevArithmetic(0,255,255,0,0,true,3,false)==-128);
 assert(tevArithmetic(0,255,255,-1024,0,false,2,false)==-1024);
 assert(tevArithmetic(0,255,255,1023,0,false,2,false)==1023);
 assert(tevArithmetic(0,0,0,0,1,false,0,false)==128);
 assert(tevArithmetic(0,0,0,0,2,false,0,false)==-128);
 unsigned checked=0;
 for(int value=0;value<256;++value)for(int c=0;c<256;++c){
  assert(tevArithmetic(value,value,c,0,0,false,0,true)==value);
  assert(tevArithmetic(255-value,value,0,0,0,false,0,true)==255-value);
  assert(tevArithmetic(255-value,value,255,0,0,false,0,true)==value);
  checked+=3;
 }
 bool rejected=false;try{tevArithmetic(0,0,0,0,3,false,0,true);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 using Native::TevRgb;
 TevRgb low={255,0,0},high={0,1,0},highest={0,0,1};
 assert(Native::tevPackedCompare(low,high,0));
 assert(!Native::tevPackedCompare(low,high,2));
 assert(Native::tevPackedCompare(high,low,2));
 assert(Native::tevPackedCompare(highest,high,4));
 for(unsigned mode=0;mode<3;++mode){assert(Native::tevPackedCompare(high,high,mode*2+1));assert(!Native::tevPackedCompare(high,high,mode*2));}
 assert((Native::tevColorCompare({2,1,3},{1,1,4},{10,20,30},{-1,2,3},6,false)==TevRgb{9,2,3}));
 assert((Native::tevColorCompare({2,1,3},{1,1,4},{10,20,30},{-1,2,3},7,false)==TevRgb{-1,22,3}));
 assert((Native::tevColorCompare(high,low,{255,255,255},{100,100,100},2,true)==TevRgb{255,255,255}));
 assert(Native::tevAlphaCompare(high,low,0,255,40,5,2,false)==45);
 assert(Native::tevAlphaCompare(high,low,0,255,40,5,6,false)==5);
 assert(Native::tevAlphaCompare(high,low,255,255,40,5,7,false)==45);
 unsigned comparisons=0;
 for(int a=0;a<256;++a)for(int b=0;b<256;++b){
  assert(Native::tevAlphaCompare({}, {},a,b,19,-3,6,false)==(a>b?16:-3));
  assert(Native::tevAlphaCompare({}, {},a,b,19,-3,7,false)==(a==b?16:-3));
  comparisons+=2;
 }
 std::printf("PASS TEV comparisons: packed RGB order, per-channel modes, alpha routing and %u cases\n",comparisons);
 std::printf("PASS normal TEV arithmetic: rounding, signed division, bias, saturation and %u endpoint checks\n",checked);
}
