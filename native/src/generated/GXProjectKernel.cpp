#include <revolution/mtx.h>
#include <revolution/gx/GXTransform.h>
extern "C" void GXProject(f32 x, f32 y, f32 z, const f32 mtx[3][4], const f32* pm, const f32* vp, f32* sx, f32* sy, f32* sz) {
    Vec peye;
    f32 xc, yc, zc, wc;

    peye.x = mtx[0][0] * x + mtx[0][1] * y + mtx[0][2] * z + mtx[0][3];
    peye.y = mtx[1][0] * x + mtx[1][1] * y + mtx[1][2] * z + mtx[1][3];
    peye.z = mtx[2][0] * x + mtx[2][1] * y + mtx[2][2] * z + mtx[2][3];

    if (pm[0] == (f32)GX_PERSPECTIVE) {
        xc = peye.x * pm[1] + peye.z * pm[2];
        yc = peye.y * pm[3] + peye.z * pm[4];
        zc = peye.z * pm[5] + pm[6];
        wc = 1.0f / -peye.z;
    } else {
        xc = peye.x * pm[1] + pm[2];
        yc = peye.y * pm[3] + pm[4];
        zc = peye.z * pm[5] + pm[6];
        wc = 1.0f;
    }

    *sx = xc * vp[2] / 2 * wc + vp[0] + vp[2] / 2;
    *sy = -yc * vp[3] / 2 * wc + vp[1] + vp[3] / 2;
    *sz = zc * (vp[5] - vp[4]) * wc + vp[5];
}


