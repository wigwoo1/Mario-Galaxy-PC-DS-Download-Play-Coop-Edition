#include <Game/Util/MathUtil.hpp>
#include <cmath>
namespace MR {
// Scalar replacement of the original paired-single weighted vector operation.
// Read both vectors before writing so original in-place animation calls work.
void PSvecBlend(const TVec3f* from,const TVec3f* to,TVec3f* destination,f32 fromWeight,f32 toWeight){
    const float x=std::fma(to->x,toWeight,from->x*fromWeight);
    const float y=std::fma(to->y,toWeight,from->y*fromWeight);
    const float z=std::fma(to->z,toWeight,from->z*fromWeight);
    destination->x=x;destination->y=y;destination->z=z;
}
}
