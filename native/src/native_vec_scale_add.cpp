#include <JSystem/JGeometry/TVec.hpp>
#include <cmath>
namespace MR {
// Original paired-single body loads both vectors before writing A += B * scale.
void vecScaleAdd(const TVec3f* a, const TVec3f* b, f32 scale) {
    const f32 x=std::fma(b->x,scale,a->x);
    const f32 y=std::fma(b->y,scale,a->y);
    const f32 z=std::fma(b->z,scale,a->z);
    const_cast<TVec3f*>(a)->set(x,y,z);
}
}
