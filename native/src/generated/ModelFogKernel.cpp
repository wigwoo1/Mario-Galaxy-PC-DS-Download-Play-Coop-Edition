#include <Game/Util/ModelUtil.hpp>
#include <Game/Util/CameraUtil.hpp>
#include <Game/Util/DirectDraw.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>
#include <revolution/mtx.h>
namespace TDDraw {
    void mixFogColor(TVec3f position, f32 radius, u32 packedColor) {
        f32 nearZ = MR::getNearZ();
        f32 farZ = MR::getFarZ();
        GXColor color;
        setGXColor(packedColor, &color);
        f32 start;
        f32 end;
        MR::calcFogStartEnd(position, radius, &start, &end);
        GXSetFog(GX_FOG_PERSP_LIN, start, end, nearZ, farZ, color);
    }
}
