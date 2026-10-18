#include <JSystem/J3DGraphBase/J3DShapeMtx.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>

// Complete the GPU/GPU branch alongside the original PCPU/NCPU/PNCPU branches.
// Both indexed emitters remain the original J3DSys methods.
void J3DShapeMtx::loadMtxIndx_PNGP(int slot,u16 index) const {
    j3dSys.loadPosMtxIndx(slot,index);
    j3dSys.loadNrmMtxIndx(slot,index);
}
