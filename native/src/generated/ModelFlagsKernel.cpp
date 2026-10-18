#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>
void J3DModelData::syncJ3DSysFlags() const {
    if (checkFlag(0x20)) {
        j3dSys.onFlag(J3DSysFlag_PostTexMtx);
    } else {
        j3dSys.offFlag(J3DSysFlag_PostTexMtx);
    }
}