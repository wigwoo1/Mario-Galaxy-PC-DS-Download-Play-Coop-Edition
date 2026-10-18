#include <Native/GxImmediateVertices.hpp>
#include <revolution/gd/GDBase.h>
#include <stdexcept>

namespace Native {
void writePositionTexCoord(float x, float y, float z, float s, float t) {
    if (!__GDCurrentDL) throw std::logic_error("No active GD display list for immediate vertex");
    GDOverflowCheck(20);
    GDWrite_f32(x);
    GDWrite_f32(y);
    GDWrite_f32(z);
    GDWrite_f32(s);
    GDWrite_f32(t);
}
}
