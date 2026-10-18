#include "Game/Util/MathUtil.hpp"
namespace MR {
    f32 getInterpolateValue(f32 t, f32 start, f32 end) {
        return start + (end - start) * t;
    }
    f32 getLinerValue(f32 x, f32 start, f32 end, f32 max) {
        return getInterpolateValue(x / max, start, end);
    }
    f32 getEaseInValue(f32 x, f32 start, f32 end, f32 max) {
        f32 t = x / max * pi() / 2.0f;
        return getInterpolateValue(1.0f - JMACosRadian(x / max * pi() / 2.0f), start, end);
    }
    f32 getEaseOutValue(f32 x, f32 start, f32 end, f32 max) {
        f32 t = x / max * pi() / 2.0f;
        return getInterpolateValue(JMASinRadian(t), start, end);
    }
    f32 getEaseInOutValue(f32 x, f32 start, f32 end, f32 max) {
        f32 t = x / max * pi();
        return getInterpolateValue((1.0f - JMACosRadian(t)) / 2.0f, start, end);
    }
    f32 normalize(f32 x, f32 min, f32 max) {
        f32 range = max - min;

        if (!isNearZero(range)) {
        } else if (x < min) {
            return 0.0f;
        } else {
            return 1.0f;
        }

        return (JGeometry::TUtil< f32 >::clamp(x, min, max) - min) / range;
    }
#ifndef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
    bool isNearZero(f32 x, f32 tolerance) {
        if (x < 0.0f) {
            x = -x;
        }

        if (x < tolerance) {
            return true;
        }

        return false;
    }
#endif
}
