// Native scalar arithmetic; finite scale-zero integer conversions clamp/truncate. NaN conversion is unsupported.
#pragma once

#include <cmath>
#include <stdexcept>
#include <revolution/types.h>

namespace nw4r {
    namespace math {
        inline f32 U16ToF32(u16 x) { return static_cast<f32>(x); }

        inline u16 F32ToU16(f32 x) { if(std::isnan(x)) throw std::domain_error("NaN NW4R integer conversion"); if(x<=0) return 0; if(x>=65535.0f) return 65535; return static_cast<u16>(x); }

        inline f32 FAbs(f32 x) { return __fabsf(x); }

        inline f32 FSelect(f32 cond, f32 ifPos, f32 ifNeg) { return cond >= 0.0f ? ifPos : ifNeg; }
    };  // namespace math
};  // namespace nw4r


