#include <revolution/gx.h>
static u32 __GXGetNumXfbLines(u32 efbHt, u32 iScale) {
    u32 count, realHt, iScaleD;

    count = (efbHt - 1) * 256;
    realHt = count / iScale + 1;

    iScaleD = iScale;
    if (iScaleD > 0x80 && iScaleD < 0x100) {
        while ((iScaleD & 0x01) == 0) {
            iScaleD >>= 1;
        }

        if ((efbHt % iScaleD) == 0) {
            ++realHt;
        }
    }

    if (realHt > 1024) {
        realHt = 1024;
    }

    return realHt;
}
u16 GXGetNumXfbLines(u16 efbHeight, f32 yScale) {
    u32 iScale;
    iScale = (u32)(256.0f / yScale) & 0x1ff;
    return ((u16)__GXGetNumXfbLines((u32)efbHeight, iScale));
}
f32 GXGetYScaleFactor(u16 efbHeight, u16 xfbHeight) {
    f32 fScale, yScale;
    u32 iScale, tgtHt, realHt;

    tgtHt = xfbHeight;
    yScale = (f32)xfbHeight / (f32)efbHeight;
    iScale = (u32)(256.0f / yScale) & 0x1ff;
    realHt = __GXGetNumXfbLines((u32)efbHeight, iScale);

    while (realHt > xfbHeight) {
        tgtHt--;
        yScale = (f32)tgtHt / (f32)efbHeight;
        iScale = (u32)(256.0f / yScale) & 0x1ff;
        realHt = __GXGetNumXfbLines((u32)efbHeight, iScale);
    }

    fScale = yScale;
    while (realHt < xfbHeight) {
        fScale = yScale;
        tgtHt++;
        yScale = (f32)tgtHt / (f32)efbHeight;
        iScale = (u32)(256.0f / yScale) & 0x1ff;
        realHt = __GXGetNumXfbLines((u32)efbHeight, iScale);
    }

    return fScale;
}

