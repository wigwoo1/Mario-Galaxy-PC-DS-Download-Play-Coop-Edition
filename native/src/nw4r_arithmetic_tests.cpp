#include "nw4r/math/arithmetic.h"
#include <cmath>
#include <limits>
int main(){
    using namespace nw4r::math;
    if(FAbs(-3.5f)!=3.5f||std::signbit(FAbs(-0.0f)))return 1;
    if(FSelect(1,3,4)!=3||FSelect(-1,3,4)!=4)return 2;
    if(FSelect(0,3,4)!=3||FSelect(-0.0f,3,4)!=3)return 3;
    if(FSelect(std::numeric_limits<float>::quiet_NaN(),3,4)!=4)return 4;
    if(FSelect(std::numeric_limits<float>::infinity(),3,4)!=3||FSelect(-std::numeric_limits<float>::infinity(),3,4)!=4)return 5;
    for(unsigned int i=0;i<=65535;++i)if(F32ToU16(U16ToF32(static_cast<u16>(i)))!=i)return 6; if(F32ToU16(-1)!=0||F32ToU16(3.9f)!=3||F32ToU16(65536)!=65535)return 7; bool rejected=false;try{F32ToU16(std::numeric_limits<float>::quiet_NaN());}catch(const std::domain_error&){rejected=true;}if(!rejected)return 8; return 0;
}

