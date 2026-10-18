#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <array>
#include <cmath>
#include <stdexcept>
namespace {
std::array<float,7> retained{};
bool loaded=false;
void emit(const std::array<float,7>& values){
 for(float value:values)if(!std::isfinite(value))throw std::invalid_argument("Nonfinite GX projection");
 GDOverflowCheck(33);GDWrite_u8(0x10);GDWrite_u32(0x61020);
 for(unsigned i=1;i<7;++i)GDWrite_f32(values[i]);GDWrite_u32(values[0]==0?0:1);
 retained=values;loaded=true;
}
}
extern "C" void GXSetProjection(const f32 matrix[4][4],GXProjectionType type){
 if(!matrix || (type!=GX_PERSPECTIVE && type!=GX_ORTHOGRAPHIC))throw std::invalid_argument("GX projection matrix/type");
 // Original GXTransform.c retains these six matrix entries, plus projection type.
 unsigned offset=type==GX_ORTHOGRAPHIC?3:2;
 emit({float(type),matrix[0][0],matrix[0][offset],matrix[1][1],matrix[1][offset],matrix[2][2],matrix[2][3]});
}
extern "C" void GXSetProjectionv(const f32* values){
 if(!values || !std::isfinite(values[0]))throw std::invalid_argument("GX projection vector");
 emit({values[0]==0?0.f:1.f,values[1],values[2],values[3],values[4],values[5],values[6]});
}
extern "C" void GXGetProjectionv(f32* values){
 if(!values)throw std::invalid_argument("GX projection destination");
 if(!loaded)throw std::logic_error("GX projection not configured");
 std::copy(retained.begin(),retained.end(),values);
}
