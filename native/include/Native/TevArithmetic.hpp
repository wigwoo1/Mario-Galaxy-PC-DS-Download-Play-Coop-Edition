#pragma once
#include <algorithm>
#include <stdexcept>
#include <array>
namespace Native {
// Normal TEV arithmetic only. Comparison operations have separate semantics.
// Fixed-point behavior checked against Dolphin's UberShaderPixel TEV lerp.
inline int tevArithmetic(int a,int b,int c,int d,unsigned bias,
                         bool subtract,unsigned scale,bool clamp){
 if(a<0 || a>255 || b<0 || b>255 || c<0 || c>255 || d<-1024 || d>1023 || bias>2 || scale>3)
  throw std::invalid_argument("Invalid normal TEV arithmetic inputs");
 auto floorDivide=[](int value,int divisor){return value>=0?value/divisor:-((-value+divisor-1)/divisor);};
 int weight=c+(c/128);
 int interpolation=a*256+(b-a)*weight;
 int base=d+(bias==1?128:bias==2?-128:0);
 if(scale!=3){int factor=1<<scale;interpolation*=factor;base*=factor;interpolation+=subtract?127:128;}
 int value=base+(subtract?-1:1)*floorDivide(interpolation,256);
 if(scale==3)value=floorDivide(value,2);
 return std::clamp(value,clamp?0:-1024,clamp?255:1023);
}
using TevRgb=std::array<int,3>;
inline bool tevPackedCompare(const TevRgb& a,const TevRgb& b,unsigned operation){
 if(operation>5)throw std::invalid_argument("Invalid packed TEV comparison");
 unsigned channels=operation/2+1;unsigned lhs=0,rhs=0;
 for(unsigned channel=0;channel<channels;++channel){
  if(a[channel]<0 || a[channel]>255 || b[channel]<0 || b[channel]>255)throw std::invalid_argument("Invalid TEV comparison input");
  lhs|=unsigned(a[channel])<<(channel*8);rhs|=unsigned(b[channel])<<(channel*8);
 }
 return operation&1?lhs==rhs:lhs>rhs;
}
inline TevRgb tevColorCompare(const TevRgb& a,const TevRgb& b,const TevRgb& c,
                             const TevRgb& d,unsigned operation,bool clamp){
 if(operation>7)throw std::invalid_argument("Invalid TEV color comparison");
 TevRgb result{};
 for(unsigned channel=0;channel<3;++channel){
  if(a[channel]<0 || a[channel]>255 || b[channel]<0 || b[channel]>255 || c[channel]<0 || c[channel]>255 || d[channel]<-1024 || d[channel]>1023)
   throw std::invalid_argument("Invalid TEV comparison channel");
  bool condition=operation<6?tevPackedCompare(a,b,operation):operation==6?a[channel]>b[channel]:a[channel]==b[channel];
  result[channel]=std::clamp(d[channel]+(condition?c[channel]:0),clamp?0:-1024,clamp?255:1023);
 }
 return result;
}
inline int tevAlphaCompare(const TevRgb& colorA,const TevRgb& colorB,int a,int b,
                          int c,int d,unsigned operation,bool clamp){
 if(operation>7 || a<0 || a>255 || b<0 || b>255 || c<0 || c>255 || d<-1024 || d>1023)
  throw std::invalid_argument("Invalid TEV alpha comparison");
 bool condition=operation<6?tevPackedCompare(colorA,colorB,operation):operation==6?a>b:a==b;
 return std::clamp(d+(condition?c:0),clamp?0:-1024,clamp?255:1023);
}
}
