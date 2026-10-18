#pragma once
#include <revolution/types.h>
extern "C" {
void GXSetViewportJitter(f32,f32,f32,f32,f32,f32,u32);
void GXGetViewportv(f32*);
void GXSetZScaleOffset(f32,f32);
}
