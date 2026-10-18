#include <initializer_list>
#include <nw4r/math/triangular.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
namespace nw4r::math::detail {extern SinCosSample gSinCosTbl[257];}
int main(){constexpr double pi=3.14159265358979323846;
for(int i=-4096;i<=4096;++i){float angle=float(i)*0.25f;auto s=nw4r::math::SinFIdx(angle),c=nw4r::math::CosFIdx(angle);assert(std::fabs(s-std::sin(double(angle)*pi/128))<0.00008 && std::fabs(c-std::cos(double(angle)*pi/128))<0.00008);float absolute=std::fabs(angle);unsigned whole=unsigned(absolute);auto sample=nw4r::math::detail::gSinCosTbl[whole&255];float fraction=absolute-whole;assert(s==(angle<0?-(sample.sin_val+fraction*sample.sin_delta):sample.sin_val+fraction*sample.sin_delta));assert(c==sample.cos_val+fraction*sample.cos_delta);}
for(float angle:{65536.25f,131072.5f,1.0e20f,std::numeric_limits<float>::max()}){assert(std::isfinite(nw4r::math::SinFIdx(angle)) && std::isfinite(nw4r::math::CosFIdx(angle)));auto reduced=std::fmod(angle,65536.0f);assert(nw4r::math::SinFIdx(angle)==nw4r::math::SinFIdx(reduced));assert(nw4r::math::CosFIdx(angle)==nw4r::math::CosFIdx(reduced));}
for(float angle:{std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){bool a=false,b=false;try{nw4r::math::SinFIdx(angle);}catch(const std::domain_error&){a=true;}try{nw4r::math::CosFIdx(angle);}catch(const std::domain_error&){b=true;}assert(a && b);}
std::puts("PASS original NW4R table interpolation: 8193 angles, bounded large-angle reduction, non-finite rejection");}

