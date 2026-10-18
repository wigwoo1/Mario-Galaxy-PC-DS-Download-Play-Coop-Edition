#include "Game/Map/KCollision.hpp"
#include "Game/Camera/CameraPolygonCodeUtil.hpp"
#include "Game/Map/CollisionCode.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/MathUtil.hpp"
#include <algorithm>
void KCollisionServer::init(void* pData, const void* pMapData) {
    setData(pData);

    if (pMapData != nullptr) {
        mapInfo->attach(pMapData);
    }
}

void KCollisionServer::setData(void* pData) {
    mFile = reinterpret_cast< KCLFile* >(pData);

    if (!isBinaryInitialized(pData)) {
        mFile->mPos = reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mFile) + mFile->mPosOffset);
        mFile->mNorms = reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mFile) + mFile->mNormOffset);
        mFile->mPrisms = reinterpret_cast< KC_PrismData* >(reinterpret_cast< u8* >(mFile) + mFile->mPrismOffset);
        mFile->mOctree = reinterpret_cast< void* >(reinterpret_cast< u8* >(mFile) + mFile->mOctreeOffset);
    }
}

bool KCollisionServer::isBinaryInitialized(const void* pData) NO_INLINE {
    return reinterpret_cast< const s32* >(pData)[0] < 0;
}

KC_PrismData* KCollisionServer::checkPoint(Fxyz* pPoint, f32 param, f32* pDist) {
    f32 maxDist = mFile->mThickness * param;
    u32 x = (s32)(pPoint->x - mFile->mMin.x);

    if (x & mFile->mXMask) {
        return nullptr;
    }

    u32 y = (s32)(pPoint->y - mFile->mMin.y);

    if (y & mFile->mYMask) {
        return nullptr;
    }

    u32 z = (s32)(pPoint->z - mFile->mMin.z);

    if (z & mFile->mZMask) {
        return nullptr;
    }

    s32 shift;
    u16* prismList = (u16*)searchBlock(&shift, x, y, z);

    while (*++prismList != 0) {
        KC_PrismData* prism = &mFile->mPrisms[*prismList];
        f32 height = prism->mHeight;

        if (height <= 0.0f) {
            continue;
        }

        TVec3f* vtx = &mFile->mPos[prism->mPositionIndex];

        Fxyz dir;
        dir.x = pPoint->x - vtx->x;
        dir.y = pPoint->y - vtx->y;
        dir.z = pPoint->z - vtx->z;

        TVec3f& edgeNormal0 = mFile->mNorms[prism->mEdgeIndices[0]];

        if (dir.x * edgeNormal0.x + dir.y * edgeNormal0.y + dir.z * edgeNormal0.z > 0.0f) {
            continue;
        }

        TVec3f& edgeNormal1 = mFile->mNorms[prism->mEdgeIndices[1]];

        if (dir.x * edgeNormal1.x + dir.y * edgeNormal1.y + dir.z * edgeNormal1.z > 0.0f) {
            continue;
        }

        TVec3f& edgeNormal2 = mFile->mNorms[prism->mEdgeIndices[2]];

        if (dir.x * edgeNormal2.x + dir.y * edgeNormal2.y + dir.z * edgeNormal2.z > height) {
            continue;
        }

        TVec3f& faceNormal = mFile->mNorms[prism->mNormalIndex];
        f32 dist = -dir.x * faceNormal.x - dir.y * faceNormal.y - dir.z * faceNormal.z;

        if (dist < 0.0f) {
            continue;
        }

        if (maxDist < dist) {
            continue;
        }

        *pDist = dist;
        return prism;
    }

    return nullptr;
}

u32 KCollisionServer::checkArea3D(Fxyz* pMin, Fxyz* pMax, KC_PrismData** pOut, u32 maxCount) {
    s32 remX;
    s32 yStep;
    s32 zStep;
    s32 bestRemY;
    u16* list;
    u16* nextSkipList = nullptr;
    u16* skipList = nullptr;
    u32 foundCount = 0;

    Fxyz box[2];
    box[0] = *pMin;
    box[1] = *pMax;

    V3u pointMin;
    V3u pointMax;
    TVec3f queryMax;
    TVec3f queryMin;
    MR::createBoundingBox((TVec3f*)box, 2, &queryMin, &queryMax);

    if (queryMin.x == queryMax.x) {
        queryMin.x -= 1.0f;
        queryMax.x += 1.0f;
    }

    if (queryMin.y == queryMax.y) {
        queryMin.y -= 1.0f;
        queryMax.y += 1.0f;
    }

    if (queryMin.z == queryMax.z) {
        queryMin.z -= 1.0f;
        queryMax.z += 1.0f;
    }

    if (!outCheck(&queryMin, &queryMax, &pointMin, &pointMax)) {
        return 0;
    }

    u32 x;
    u32 y;
    u32 z;
    KC_PrismData* prism;
    s32 shift;
    z = pointMin.z;

    do {
        y = pointMin.y;
        zStep = 1000000;

        do {
            x = pointMin.x;
            yStep = 1000000;
            bestRemY = 0;

            do {
                list = reinterpret_cast< u16* >(searchBlock(&shift, x, y, z));
                shift = 1 << shift;
                s32 blockSize = shift;
                s32 remZ;
                s32 mask = blockSize - 1;
                remX = blockSize - (x & mask);
                s32 remY = blockSize - (y & mask);
                remZ = blockSize - (z & mask);

                if (remZ < zStep) {
                    zStep = remZ;
                }

                if (remY < yStep) {
                    yStep = remY;
                }

                if (remY > bestRemY && list[1] != 0) {
                    bestRemY = remY;
                    nextSkipList = list;
                }

                if (skipList == nullptr || list != skipList) {
                    while (*++list != 0) {
                        prism = &mFile->mPrisms[*list];

                        if (prism->mHeight <= 0.0f) {
                            continue;
                        }

                        KC_PrismData** end = pOut + foundCount;

                        if (std::find(pOut, end, (KC_PrismData* const&)prism) != end) {
                            continue;
                        }

                        TVec3f verts[3];
                        verts[0] = getPos(prism, 0);
                        verts[1] = getPos(prism, 1);
                        verts[2] = getPos(prism, 2);

                        TVec3f prismMin;
                        TVec3f prismMax;
                        MR::createBoundingBox(verts, 3, &prismMin, &prismMax);

                        if (prismMax.x < queryMin.x) {
                            continue;
                        }

                        if (prismMax.y < queryMin.y) {
                            continue;
                        }

                        if (prismMax.z < queryMin.z) {
                            continue;
                        }

                        if (queryMax.x < prismMin.x) {
                            continue;
                        }

                        if (queryMax.y < prismMin.y) {
                            continue;
                        }

                        if (queryMax.z < prismMin.z) {
                            continue;
                        }

                        *end = prism;
                        foundCount++;

                        if (foundCount == maxCount) {
                            return maxCount;
                        }
                    }
                }

                x += remX;
            } while (x <= pointMax.x);

            skipList = nextSkipList;
            y += yStep;
        } while (y <= pointMax.y);

        z += zStep;
    } while (z <= pointMax.z);

    return foundCount;
}

u32 KCollisionServer::checkSphereWithThickness(Fxyz* pCenter, f32 radius, f32 param, u32 maxCount, KC_PrismData** pOut, f32* pDists, u8* pFlags,
                                               f32 thickness) {
    KC_PrismData** end;
    s32 remX;
    s32 yStep;
    s32 zStep;
    s32 bestRemY;
    u16* list;
    u16* nextSkipList = nullptr;
    u16* skipList = nullptr;
    u32 foundCount = 0;

    V3u pointMin;
    V3u pointMax;
    TVec3f boxMax;
    TVec3f boxMin;
    u32 x;
    u32 y;
    u32 z;
    KC_PrismData* prism;
    f32 dist = 0.0f;
    s32 shift;
    u8 flag;
    boxMax.x = pCenter->x + radius;
    boxMax.y = pCenter->y + radius;
    boxMax.z = pCenter->z + radius;

    boxMin.x = pCenter->x - radius;
    boxMin.y = pCenter->y - radius;
    boxMin.z = pCenter->z - radius;

    if (!outCheck(&boxMin, &boxMax, &pointMin, &pointMax)) {
        return 0;
    }

    z = pointMin.z;

    do {
        y = pointMin.y;
        zStep = 1000000;

        do {
            x = pointMin.x;
            yStep = 1000000;
            bestRemY = 0;

            do {
                list = reinterpret_cast< u16* >(searchBlock(&shift, x, y, z));
                s32 remZ;
                s32 blockSize = 1 << shift;
                s32 mask = blockSize - 1;
                remX = blockSize - (x & mask);
                s32 remY = blockSize - (y & mask);
                remZ = blockSize - (z & mask);

                if (remZ < zStep) {
                    zStep = remZ;
                }

                if (remY < yStep) {
                    yStep = remY;
                }

                if (remY > bestRemY && list[1] != 0) {
                    bestRemY = remY;
                    nextSkipList = list;
                }

                if (skipList == nullptr || list != skipList) {
                    while (*++list != 0) {
                        prism = &mFile->mPrisms[*list];

                        if (prism->mHeight <= 0.0f) {
                            continue;
                        }

                        end = pOut + foundCount;

                        if (std::find(pOut, end, (KC_PrismData* const&)prism) != end) {
                            continue;
                        }

                        if (!KCHitSphereWithThickness(prism, pCenter, radius, param, &dist, &flag, thickness)) {
                            continue;
                        }

                        if (foundCount >= maxCount) {
                            continue;
                        }

                        if (std::find(pOut, end, (KC_PrismData* const&)prism) != end) {
                            continue;
                        }

                        *end = prism;
                        pDists[foundCount] = dist;
                        pFlags[foundCount] = flag;
                        foundCount++;
                    }
                }

                x += remX;
            } while (x <= pointMax.x);

            skipList = nextSkipList;
            y += yStep;
        } while (y <= pointMax.y);

        z += zStep;
    } while (z <= pointMax.z);

    return foundCount;
}

bool KCollisionServer::KCHitSphereWithThickness(KC_PrismData* pPrism, Fxyz* pCenter, f32 radius, f32 param, f32* pDist, u8* pFlag, f32 thickness) {
    f32 distance;
    f32 radiusSq = radius * radius;
    f32 threshold = thickness * param;
    *pFlag = 0;

    TVec3f* v0 = &mFile->mPos[pPrism->mPositionIndex];

    Fxyz dir;
    f32 distances[4];
    dir.x = pCenter->x - v0->x;
    dir.y = pCenter->y - v0->y;
    dir.z = pCenter->z - v0->z;

    TVec3f* n0 = &mFile->mNorms[pPrism->mEdgeIndices[0]];
    distances[1] = dir.x * n0->x + dir.y * n0->y + dir.z * n0->z;

    if (distances[1] >= radius) {
        return false;
    }

    TVec3f* n1 = &mFile->mNorms[pPrism->mEdgeIndices[1]];
    distances[2] = dir.x * n1->x + dir.y * n1->y + dir.z * n1->z;

    if (distances[2] >= radius) {
        return false;
    }

    TVec3f* n2 = &mFile->mNorms[pPrism->mEdgeIndices[2]];
    distances[3] = dir.x * n2->x + dir.y * n2->y + dir.z * n2->z - pPrism->mHeight;

    if (distances[3] >= radius) {
        return false;
    }

    TVec3f* faceNormal = &mFile->mNorms[pPrism->mNormalIndex];
    distances[0] = dir.x * faceNormal->x + dir.y * faceNormal->y + dir.z * faceNormal->z;
    *pDist = radius - distances[0];

    if (*pDist < 0.0f) {
        return false;
    }

    f32 nn;

    if (distances[1] > distances[2]) {
        if (!(distances[1] > distances[3])) {
            goto side2;
        }
        goto side0;
    } else {
        if (!(distances[2] > distances[3])) {
            goto side2;
        }
        goto side1;
    }

side0:
    if (distances[1] <= 0.0f) {
        if (threshold < *pDist) {
            return false;
        }

        *pFlag = 1;
        goto success;
    }

    if (distances[2] > distances[3]) {
        nn = n0->x * n1->x + n0->y * n1->y + n0->z * n1->z;

        if (!(nn * distances[1] > distances[2])) {
            goto region5;
        }

        goto vertex2;
    }

    nn = n0->x * n2->x + n0->y * n2->y + n0->z * n2->z;

    if (!(nn * distances[1] > distances[3])) {
        goto region7;
    }

    goto vertex2;

side1:
    if (distances[2] <= 0.0f) {
        if (threshold < *pDist) {
            return false;
        }

        *pFlag = 1;
        goto success;
    }

    if (distances[3] > distances[1]) {
        nn = n1->x * n2->x + n1->y * n2->y + n1->z * n2->z;

        if (!(nn * distances[2] > distances[3])) {
            goto region6;
        }

        goto vertex3;
    }

    nn = n1->x * n0->x + n1->y * n0->y + n1->z * n0->z;

    if (!(nn * distances[2] > distances[1])) {
        goto region5;
    }

    goto vertex3;

side2:

    if (distances[3] <= 0.0f) {
        if (threshold < *pDist) {
            return false;
        }

        *pFlag = 1;
        goto success;
    }

    if (distances[1] > distances[2]) {
        nn = n2->x * n0->x + n2->y * n0->y + n2->z * n0->z;

        if (!(nn * distances[3] > distances[1])) {
            goto region7;
        }

        goto vertex4;
    }

    nn = n2->x * n1->x + n2->y * n1->y + n2->z * n1->z;

    if (!(nn * distances[3] > distances[2])) {
        goto region6;
    }

    goto vertex4;

vertex2:
    *pDist = radiusSq - distances[1] * distances[1];
    *pFlag = 2;
    goto finish;

vertex3:
    *pDist = radiusSq - distances[2] * distances[2];
    *pFlag = 3;
    goto finish;

vertex4:
    *pDist = radiusSq - distances[3] * distances[3];
    *pFlag = 4;
    goto finish;

region5: {
    f32 t = (nn * distances[2] - distances[1]) / (nn * nn - 1.0f);
    f32 s = distances[2] - t * nn;
    dir.x = t * n0->x + s * n1->x;
    dir.y = t * n0->y + s * n1->y;
    dir.z = t * n0->z + s * n1->z;
    *pFlag = 5;
    goto edgeFinish;
}

region6: {
    f32 t = (nn * distances[3] - distances[2]) / (nn * nn - 1.0f);
    f32 s = distances[3] - t * nn;
    dir.x = t * n1->x + s * n2->x;
    dir.y = t * n1->y + s * n2->y;
    dir.z = t * n1->z + s * n2->z;
    *pFlag = 6;
    goto edgeFinish;
}

region7: {
    f32 t = (nn * distances[1] - distances[3]) / (nn * nn - 1.0f);
    f32 s = distances[1] - t * nn;
    dir.x = t * n2->x + s * n0->x;
    dir.y = t * n2->y + s * n0->y;
    dir.z = t * n2->z + s * n0->z;
    *pFlag = 7;
}

edgeFinish: {
    f32 closestSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    f32 dist = MR::sqrt(closestSq);

    if (dist >= radius) {
        *pFlag = 0;
        return false;
    }

    *pDist = radiusSq - closestSq;
}

finish:
    distance = MR::sqrt(*pDist);

    if (distances[0] + distance < 0.0f) {
        *pFlag = 0;
        return false;
    }

    *pDist = distance - distances[0];

    if (*pDist < 0.0f || threshold < *pDist) {
        *pFlag = 0;
        return false;
    }

success:
    return true;
}

Fxyz& Fxyz::operator=(const Fxyz& rOther) {
    x = rOther.x;
    y = rOther.y;
    z = rOther.z;

    return *this;
}