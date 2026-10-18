#include <nw4r/lyt/common.h>
#include <nw4r/math/types.h>
#include <nw4r/ut/Color.h>
namespace nw4r::lyt::detail {
            void DrawQuad(const math::VEC2& basePt, const Size& size, u8 num, const math::VEC2 (*texCoords)[4], const ut::Color* vtxColors) {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition2f32(basePt.x, basePt.y);

                if (vtxColors) {
                    GXColor1u32(vtxColors[0]);
                }

                for (int i = 0; i < num; i++) {
                    GXTexCoord2f32(texCoords[i][0].x, texCoords[i][0].y);
                }

                GXPosition2f32(basePt.x + size.width, basePt.y);

                if (vtxColors) {
                    GXColor1u32(vtxColors[1]);
                }

                for (int i = 0; i < num; i++) {
                    GXTexCoord2f32(texCoords[i][1].x, texCoords[i][1].y);
                }

                GXPosition2f32(basePt.x + size.width, basePt.y + size.height);

                if (vtxColors) {
                    GXColor1u32(vtxColors[3]);
                }

                for (int i = 0; i < num; i++) {
                    GXTexCoord2f32(texCoords[i][3].x, texCoords[i][3].y);
                }

                GXPosition2f32(basePt.x, basePt.y + size.height);

                if (vtxColors) {
                    GXColor1u32(vtxColors[2]);
                }

                for (int i = 0; i < num; i++) {
                    GXTexCoord2f32(texCoords[i][2].x, texCoords[i][2].y);
                }

                GXEnd();
            }


}
