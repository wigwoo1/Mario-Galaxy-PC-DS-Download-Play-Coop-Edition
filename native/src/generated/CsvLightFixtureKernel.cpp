#include <revolution/gx/GXLighting.h>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <cstdio>
void MR::getCsvDataStr(const char** pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue(idx, pKey, pOut);
}
void MR::getCsvDataS32(s32* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue< s32 >(idx, pKey, pOut);
}
void MR::getCsvDataU8(u8* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    s32 val = 0;
    pMapInfo->getValue< s32 >(idx, pKey, &val);
    *pOut = val;
}
void MR::getCsvDataF32(f32* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue(idx, pKey, pOut);
}
void MR::getCsvDataBool(bool* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    pMapInfo->getValue(idx, pKey, pOut);
}
void MR::getCsvDataVec(Vec* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    char key[256];
    snprintf(key, sizeof(key), "%sX", pKey);
    getCsvDataF32(&pOut->x, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sY", pKey);
    getCsvDataF32(&pOut->y, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sZ", pKey);
    getCsvDataF32(&pOut->z, pMapInfo, key, idx);
}
void MR::getCsvDataColor(GXColor* pOut, const JMapInfo* pMapInfo, const char* pKey, s32 idx) {
    char key[256];
    snprintf(key, sizeof(key), "%sR", pKey);
    getCsvDataU8(&pOut->r, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sG", pKey);
    getCsvDataU8(&pOut->g, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sB", pKey);
    getCsvDataU8(&pOut->b, pMapInfo, key, idx);
    snprintf(key, sizeof(key), "%sA", pKey);
    getCsvDataU8(&pOut->a, pMapInfo, key, idx);
}
