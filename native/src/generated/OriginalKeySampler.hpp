#pragma once
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/JMath/JMath.hpp>
inline f32 J3DHermiteInterpolation(f32 p1, f32 const* p2, f32 const* p3, f32 const* p4, f32 const* p5, f32 const* p6, f32 const* p7) {
    return JMAHermiteInterpolation(p1, *p2, *p3, *p4, *p5, *p6, *p7);
}

inline f32 J3DHermiteInterpolation(__REGISTER f32 pp1, __REGISTER s16 const* pp2, __REGISTER s16 const* pp3, __REGISTER s16 const* pp4,
                                   __REGISTER s16 const* pp5, __REGISTER s16 const* pp6, __REGISTER s16 const* pp7) {
#ifdef __MWERKS__
    register f32 value = pp1;
    register f32 time;
    register f32 start;
    register f32 end;
    register f32 duration;
    register f32 delta;
    register f32 t;
    register f32 squared;
    asm {
        psq_l time, 0(pp2), 1, 5
        psq_l end, 0(pp5), 1, 5
        psq_l start, 0(pp3), 1, 5
        fsubs duration, end, time
        psq_l end, 0(pp6), 1, 5
        fsubs t, value, time
        psq_l value, 0(pp7), 1, 5
        fsubs delta, end, start
        fdivs t, t, duration
        psq_l time, 0(pp4), 1, 5
        fmadds value, value, duration, start
        fnmsubs delta, duration, time, delta
        fmuls squared, t, t
        fsubs value, value, end
        fsubs value, value, delta
        fmuls end, squared, value
        fmadds value, duration, time, end
        fmadds value, value, t, start
        fmadds value, delta, squared, value
        fsubs value, value, end
    }

    return value;
#else
    return JMAHermiteInterpolation(pp1, *pp2, *pp3, *pp4, *pp5, *pp6, *pp7);
#endif
}

template < typename T >
f32 J3DGetKeyFrameInterpolation(f32 frame, J3DAnmKeyTableBase* pKeyTable, T* pData) {
    if (frame < pData[0]) {
        return pData[1];
    }

    if (pKeyTable->mType == 0) {
        u32 idx = (pKeyTable->mMaxFrame - 1) * 3;
        if (pData[idx] <= frame) {
            return pData[idx + 1];
        }

        u32 uVar7 = pKeyTable->mMaxFrame;
        while (uVar7 > 1) {
            u32 uVar2 = uVar7 >> 1;
            u32 tmp = uVar2 * 3;
            if (frame >= pData[tmp]) {
                pData += tmp;
                uVar7 = uVar7 - uVar2;
            } else {
                uVar7 = uVar2;
            }
        }

        f32 interpolated = J3DHermiteInterpolation(frame, &pData[0], &pData[1], &pData[2], &pData[3], &pData[4], &pData[5]);
        return interpolated;
    } else {
        u32 idx = pKeyTable->mMaxFrame - 1;
        if (pData[idx * 4] <= frame) {
            return pData[idx * 4 + 1];
        }

        u32 var_r27 = pKeyTable->mMaxFrame;
        while (var_r27 > 1) {
            u32 var_r25 = var_r27 >> 1;
            u32 var_r23 = var_r25 * 4;
            if (frame >= pData[var_r23]) {
                pData += var_r23;
                var_r27 = var_r27 - var_r25;
            } else {
                var_r27 = var_r25;
            }
        }

        f32 interpolated = J3DHermiteInterpolation(frame, &pData[0], &pData[1], &pData[3], &pData[4], &pData[5], &pData[6]);
        return interpolated;
    }
}


