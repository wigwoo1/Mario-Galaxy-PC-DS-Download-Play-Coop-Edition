#pragma once

#include <revolution/gd.h>
#include <revolution/gx/GXVert.h>

inline void J3DFifoLoadXFCmdHdr(u16 addr, u8 len) {
    GDWrite_u8(0x10);
    GDWrite_u16(len - 1);
    GDWrite_u16(addr);
}

inline void J3DFifoLoadCPCmd(u8 reg, u32 value) {
    GDWrite_u8(0x08);
    GDWrite_u8(reg);
    GDWrite_u32(value);
}

inline void J3DFifoLoadBPCmd(u32 regval) {
    GDWrite_u8(0x61);
    GDWrite_u32(regval);
}

inline void J3DFifoWriteXFCmdHdr(u16 addr, u8 len) {
    GDWrite_u8(0x10);
    GDWrite_u16(len - 1);
    GDWrite_u16(addr);
}

inline void J3DFifoLoadIndx(u8 cmd, u16 indx, u16 addr) {
    GDWrite_u8(cmd);
    GDWrite_u16(indx);
    GDWrite_u16(addr);
}

inline void J3DFifoLoadNrmMtxIndx3x3(u16 index, u32 addr) {
    GDWrite_u8(0x28);
    GDWrite_u16(index);
    GDWrite_u16(((9 - 1) << 12) | (u16)((addr * 3) + 0x400));
}

void J3DFifoLoadPosMtxImm(f32 (*)[4], u32);
void J3DFifoLoadNrmMtxImm(f32 (*)[4], u32);
void J3DFifoLoadNrmMtxImm3x3(f32 (*)[3], u32);
void J3DFifoLoadNrmMtxToTexMtx(f32 (*)[4], u32);
void J3DFifoLoadNrmMtxToTexMtx3x3(f32 (*)[3], u32);
void J3DFifoLoadTexCached(GXTexMapID, u32, GXTexCacheSize, u32, GXTexCacheSize);

