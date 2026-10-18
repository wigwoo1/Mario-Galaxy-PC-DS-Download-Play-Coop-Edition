#include <revolution/types.h>
namespace MR {
    f32 getInterpolateValue(f32 t, f32 start, f32 end) {
        return start + (end - start) * t;
    }
    f32 getLinerValue(f32 x, f32 start, f32 end, f32 max) {
        return getInterpolateValue(x / max, start, end);
    }
}
