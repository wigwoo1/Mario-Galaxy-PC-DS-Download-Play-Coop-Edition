#include <Native/GpuResources.hpp>
#include <revolution/gd/GDGeometry.h>
#include <revolution/gd/GDBase.h>
#include <revolution/gx/GXGeometry.h>
#include <stdexcept>

static unsigned arrayIndex(GXAttr attr) {
    if(attr==GX_VA_NBT)attr=GX_VA_NRM;
    if(attr<GX_VA_POS || attr>GX_LIGHT_ARRAY)
        throw std::invalid_argument("GX attribute cannot bind an array");
    return unsigned(attr-GX_VA_POS);
}
extern "C" {
#ifndef GALAXY_ORIGINAL_GD_GEOMETRY
void GDSetArrayRaw(GXAttr attr,u32 address,u8 stride) {
    const auto index=arrayIndex(attr);
    GDOverflowCheck(12);
    GDWriteCPCmd(0xa0+index,address);
    GDWriteCPCmd(0xb0+index,stride);
}
void GDSetArray(GXAttr attr,const void* data,u8 stride) {
    arrayIndex(attr);
    GDSetArrayRaw(attr,Native::gpuResources().byteAddress(data),stride);
}
#endif
void GXSetArray(GXAttr attr,const void* data,u8 stride) {
    GDSetArray(attr,data,stride);
}
void GXCallDisplayList(const void* data,u32 size) {
    const auto address=Native::gpuResources().byteAddress(data);
    Native::gpuResources().resolve(address,size);
    GDOverflowCheck(9);
    GDWrite_u8(0x40);GDWrite_u32(address);GDWrite_u32(size);
}
}

