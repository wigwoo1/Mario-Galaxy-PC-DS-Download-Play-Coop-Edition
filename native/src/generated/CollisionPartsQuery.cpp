#include <Game/Map/CollisionParts.hpp>
#include <Game/Map/HitInfo.hpp>
#include <Game/Map/KCollision.hpp>
#include <Game/Util/MathUtil.hpp>
#include <Game/Util/TriangleFilter.hpp>
#include <Game/LiveActor/Binder.hpp>
#include <Game/Util/MapUtil.hpp>
#include <Game/Map/CollisionCategorizedKeeper.hpp>
#include <Game/Util/MtxUtil.hpp>
CollisionParts::CollisionParts()
    : _0(), mHitSensor(), _CC(), _CD(true), _CE(), _CF(), _D0(), _D4(), _D8(-1.0f), _DC(1.0f), mKeeperIndex(-1), mZone() {
    mServer = new KCollisionServer();

    mPrevBaseMatrix.identity();
    mBaseMatrix.identity();
    mMatrix.identity();
    PSMTXInverse(mBaseMatrix.toMtxPtr(), mInvBaseMatrix.toMtxPtr());
}
u32 CollisionParts::checkStrikeLine(HitInfo* pHitInfo, u32 maxHits, const TVec3f& rPos, const TVec3f& rDirection, const TriangleFilterBase* pFilter) {
    f32 distances[64];
    KC_PrismData* prisms[64];
    u8 classifications[64];
    f32 length = rDirection.length();
    TVec3f localPos;
    TVec3f localDirection;
    mInvBaseMatrix.mult(rPos, localPos);
    mInvBaseMatrix.mult(rPos + rDirection, localDirection);
    localDirection = localDirection - localPos;
    u32 count = 0;
    mServer->checkArrow(localPos, localDirection, distances, classifications, &count, prisms, maxHits);
    u32 hitCount = 0;

    for (u32 i = 0; i < count; i++) {
        HitInfo& hit = pHitInfo[hitCount];
        TVec3f position(localPos + localDirection * distances[i]);
        mBaseMatrix.mult(position, position);
        hit.mParentTriangle.fillData(this, mServer->toIndex(prisms[i]), mHitSensor);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(&hit.mParentTriangle)) {
            continue;
        }

        hit._60 = length * distances[i];
        hit.mHitPos = position;
        hit._88 = classifications[i];
        hitCount++;
    }

    return hitCount;
}
u32 CollisionParts::checkStrikeBall(HitInfo* pHitInfo, u32 maxHits, const TVec3f& rPos, f32 radius, bool checkMovement,
                                    const TriangleFilterBase* pFilter) {
    KC_PrismData* prisms[64];
    f32 distances[64];
    u8 classifications[64];
    TPos3f inversePrev;
    TVec3f localPos;
    mInvBaseMatrix.mult(rPos, localPos);
    TVec3f scale;
    mInvBaseMatrix.getScale(scale);
    f32 averageScale = (scale.x + scale.y + scale.z) / 3.0f;
    f32 inverseScale = 1.0f / averageScale;
    radius *= averageScale;
    TVec3f movement(0, 0, 0);

    if (checkMovement && _D4 == 0) {
        PSMTXInverse(mPrevBaseMatrix.toMtxPtr(), inversePrev.toMtxPtr());
        TVec3f prevPos;
        inversePrev.mult(rPos, prevPos);
        TVec3f localMovement(localPos - prevPos);
        TVec3f worldMovement(localMovement);
        mBaseMatrix.mult33(worldMovement, worldMovement);
        s32 steps = static_cast< s32 >((1.0f / 35.0f) * localMovement.length()) + 1;
        TVec3f step(localMovement);

        if (steps > 1) {
            step /= steps;
        }

        TVec3f offset;
        offset.zero();

        for (s32 i = 0; i <= steps; i++) {
            movement.set(-(localMovement - offset));
            mBaseMatrix.mult33(movement, movement);
            const TVec3f* direction = i == steps ? nullptr : &worldMovement;

            u32 count = checkStrikeBallCore(pHitInfo, maxHits, prevPos + offset, movement, radius, averageScale, inverseScale, prisms, distances,
                                            classifications, pFilter, direction);
            if (count != 0) {
                return count;
            }

            offset += step;
        }

        return 0;
    } else {
        return checkStrikeBallCore(pHitInfo, maxHits, localPos, TVec3f(0, 0, 0), radius, averageScale, inverseScale, prisms, distances,
                                   classifications, pFilter, nullptr);
    }
}
u32 CollisionParts::checkStrikeBallCore(HitInfo* pHitInfo, u32 maxHits, const TVec3f& rPos, const TVec3f& rMovement, f32 radius, f32 scale,
                                        f32 inverseScale, KC_PrismData** pPrisms, f32* pDistances, u8* pClassifications,
                                        const TriangleFilterBase* pFilter, const TVec3f* pDirection) {
    const TVec3f* normal;
    u32 count =
        mServer->checkSphere(reinterpret_cast< Fxyz* >(const_cast< TVec3f* >(&rPos)), radius, scale, maxHits, pPrisms, pDistances, pClassifications);
    u32 hitCount = 0;

    for (u32 i = 0; i < count; i++) {
        HitInfo& hit = pHitInfo[hitCount];
        TVec3f position(rPos);
        calcCollidePosition(&position, *pPrisms[i], pClassifications[i]);
        mBaseMatrix.mult(position, hit.mHitPos);
        hit.mParentTriangle.fillData(this, mServer->toIndex(pPrisms[i]), mHitSensor);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(&hit.mParentTriangle)) {
            continue;
        }

        if (pDirection != nullptr && 0.0f < pDirection->dot(*hit.mParentTriangle.getFaceNormal())) {
            continue;
        }

        hit._60 = inverseScale * pDistances[i];
        hit._88 = pClassifications[i];
        hit._7C.set(rMovement);
        normal = hit.mParentTriangle.getFaceNormal();
        hit._7C.scale(normal->dot(hit._7C), *normal);
        hitCount++;
    }

    return hitCount;
}
void CollisionParts::calcCollidePosition(TVec3f* pPos, const KC_PrismData& rPrism, u8 classification) {
    TVec3f offset;
    TVec3f normal;

    switch (classification) {
    // face, project into the triangle's plane
    case 1:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        break;
    // edge 0, between verts 0-2
    case 2:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        normal.set(*mServer->getNormal(rPrism.mEdgeIndices[0]));
        offset.set(*pPos);
        offset -= mServer->getPos(&rPrism, 0);
        *pPos += -normal * offset.dot(normal);
        break;
    // edge 1, between verts 0-1
    case 3:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        normal.set(*mServer->getNormal(rPrism.mEdgeIndices[1]));
        offset.set(*pPos);
        offset -= mServer->getPos(&rPrism, 0);
        *pPos += -normal * offset.dot(normal);
        break;
    // edge 2, between verts 1-2
    case 4:
        projectToPlane(pPos, *pPos, mServer->getPos(&rPrism, 0), *mServer->getNormal(rPrism.mNormalIndex));
        normal.set(*mServer->getNormal(rPrism.mEdgeIndices[2]));
        offset.set(*pPos);
        offset -= mServer->getPos(&rPrism, 1);
        *pPos += -normal * offset.dot(normal);
        break;
    // vertex 0
    case 5:
        pPos->set(mServer->getPos(&rPrism, 0));
        break;
    // vertex 1
    case 6:
        pPos->set(mServer->getPos(&rPrism, 1));
        break;
    // vertex 2
    case 7:
        pPos->set(mServer->getPos(&rPrism, 2));
        break;
    }
}
void CollisionParts::projectToPlane(TVec3f* pProjected, const TVec3f& rPos, const TVec3f& rOrigin, const TVec3f& rNormal) {
    TVec3f projected = rPos;

    f32 distance = (rPos - rOrigin).dot(rNormal);

    projected.add(-rNormal * distance);
    pProjected->set(projected);
}
void CollisionParts::updateMtx() {
    bool update = false;

    if (_CD || _CE) {
        update = true;
    }

    if (!update) {
        if (MR::isSameMtx(mMatrix.toMtxPtr(), mBaseMatrix.toMtxPtr())) {
            _D4++;
        }
    } else {
        if (MR::isSameMtx(mMatrix.toMtxPtr(), mBaseMatrix.toMtxPtr())) {
            _D4++;
        } else {
            if (_CE) {
                _D4 = 1;
            } else {
                _D4 = 0;
            }

            f32 scale = makeEqualScale(mMatrix.toMtxPtr());
            _E8 = scale;
            f32 scaleChange = scale - _DC;
            _EC = scale;
            _F0 = scale;

            if (!MR::isNearZero(scaleChange)) {
                updateBoundingSphereRangePrivate(scale);
            }
        }

        _CE = false;

        if (_D4 < 2) {
            mPrevBaseMatrix.setInline(mBaseMatrix);
            mBaseMatrix.setInline(mMatrix);
            PSMTXInverse(mBaseMatrix.toMtxPtr(), mInvBaseMatrix.toMtxPtr());
        }
    }
}
f32 CollisionParts::makeEqualScale(MtxPtr pMatrix) {
    TPos3f& mtx = *reinterpret_cast< TPos3f* >(pMatrix);

    TVec3f scale;
    mtx.getScale(scale);

    TVec3f scaleDiff(scale.x - scale.y, scale.y - scale.z, scale.z - scale.x);

    if (MR::isNearZero(scaleDiff.x) && MR::isNearZero(scaleDiff.y) && MR::isNearZero(scaleDiff.z)) {
        return scale.x;
    }

    f32 uniformScale = 1.0f;
    TVec3f invScale;

    if (_D0) {
        invScale.set< f32 >(uniformScale / scale.x, uniformScale / scale.y, uniformScale / scale.z);
        uniformScale = 1.0f;
    } else if (_CF) {
        uniformScale = (scale.x + scale.y + scale.z) / 3.0f;
        invScale.set< f32 >(uniformScale / scale.x, uniformScale / scale.y, uniformScale / scale.z);
    }

    mtx.mMtx[0][0] *= invScale.x;
    mtx.mMtx[1][0] *= invScale.x;
    mtx.mMtx[2][0] *= invScale.x;

    mtx.mMtx[0][1] *= invScale.y;
    mtx.mMtx[1][1] *= invScale.y;
    mtx.mMtx[2][1] *= invScale.y;

    mtx.mMtx[0][2] *= invScale.z;
    mtx.mMtx[1][2] *= invScale.z;
    mtx.mMtx[2][2] *= invScale.z;

    return uniformScale;
}
void CollisionParts::setMtx(const TPos3f& rMatrix) {
    mMatrix.setInline(rMatrix);
}
void CollisionParts::setMtx() {
    mMatrix.setInline(_0);
}
void CollisionParts::updateBoundingSphereRange() {
    TPos3f matrix(_0);
    f32 scale = makeEqualScale(matrix.toMtxPtr());
    updateBoundingSphereRangePrivate(scale);
}
void CollisionParts::updateBoundingSphereRangePrivate(f32 scale) {
    _DC = scale;
    _D8 = scale * mServer->mMaxVertexDistance;
}
CollisionZone::CollisionZone(s32 zoneID) : mZoneID(zoneID), mCenter(0, 0, 0), mRadius(), mMin(0, 0, 0), mMax(0, 0, 0) {
}
void CollisionZone::addParts(CollisionParts* pParts) {
    mParts.push_back(pParts);

    if (mZoneID != 0) {
        calcMinMaxAndRadius();
    }
}
void CollisionZone::calcMinMaxAndRadius() {
    CollisionParts** pCurrent;
    mMin.zero();
    mMax.zero();
    mRadius = 0.0f;

    for (CollisionParts** pParts = mParts.begin(); pParts != mParts.end(); pParts++) {
        TVec3f min((*pParts)->getTrans());
        TVec3f max((*pParts)->getTrans());
        f32 radius = (*pParts)->_D8;
        min -= TVec3f(radius, radius, radius);
        max += TVec3f(radius, radius, radius);
        addAndUpdateMinMax(min, max);
    }

    mCenter = (mMax + mMin) / 2.0f;
    f32 radius = 0.0f;

    for (pCurrent = mParts.begin(); pCurrent != mParts.end(); pCurrent++) {
        TVec3f position((*pCurrent)->getTrans());
        position -= mCenter;
        f32 outer = position.length();
        outer += (*pCurrent)->_D8;

        if (radius < outer) {
            radius = outer;
        }
    }

    mRadius = radius;
}
void CollisionZone::addAndUpdateMinMax(TVec3f min, TVec3f max) {
    if (mRadius == 0.0f) {
        mRadius = 0.1f;
        mMin.set(min);
        mMax.set(max);
    } else {
        if (min.x < mMin.x) {
            mMin.x = min.x;
        }

        if (min.y < mMin.y) {
            mMin.y = min.y;
        }

        if (min.z < mMin.z) {
            mMin.z = min.z;
        }

        if (mMax.x < max.x) {
            mMax.x = max.x;
        }

        if (mMax.y < max.y) {
            mMax.y = max.y;
        }

        if (mMax.z < max.z) {
            mMax.z = max.z;
        }
    }
}
void CollisionZone::eraseParts(CollisionParts* pParts) {
    CollisionParts** pFound = mParts.begin();
    s32 count = mParts.size();
    CollisionParts** pEnd = mParts.begin() + count;
    while (pFound != pEnd && *pFound != pParts) {
        pFound++;
    }

    if (pFound != pEnd) {
        mParts[pFound - mParts.begin()] = mParts[count - 1];
        mParts.pop_back();
    }
}
TVec3f CollisionParts::getTrans() {
    TVec3f translation;
    mBaseMatrix.getTrans(translation);
    return translation;
}
Triangle::Triangle() {
    mParts = nullptr;
    mIdx = 0xFFFFFFFF;
    mSensor = nullptr;
    mNormals[0].zero();
    mNormals[1].zero();
    mNormals[2].zero();
    mNormals[3].zero();
    mPos[0].zero();
    mPos[1].zero();
    mPos[2].zero();
}
void Triangle::fillData(CollisionParts* pParts, u32 index, HitSensor* pSensor) {
    mParts = pParts;
    mIdx = index;
    mSensor = pSensor;

    KCollisionServer* pServer = pParts->mServer;
    TPos3f* pMatrix;
    KC_PrismData* pPrism = pServer->getPrismData(index);

    mNormals[0].set(*pServer->getFaceNormal(pPrism));
    mNormals[1].set(*pServer->getEdgeNormal1(pPrism));
    mNormals[2].set(*pServer->getEdgeNormal2(pPrism));
    mNormals[3].set(*pServer->getEdgeNormal3(pPrism));

    pMatrix = &mParts->mBaseMatrix;

    PSMTXMultVecSR(pMatrix->mMtx, &mNormals[0], &mNormals[0]);
    PSMTXMultVecSR(pMatrix->mMtx, &mNormals[1], &mNormals[1]);
    PSMTXMultVecSR(pMatrix->mMtx, &mNormals[2], &mNormals[2]);
    PSMTXMultVecSR(pMatrix->mMtx, &mNormals[3], &mNormals[3]);

    MR::normalize(&mNormals[0]);
    MR::normalize(&mNormals[1]);
    MR::normalize(&mNormals[2]);
    MR::normalize(&mNormals[3]);

    mPos[0].set(pServer->getPos(pPrism, 0));
    mPos[1].set(pServer->getPos(pPrism, 1));
    mPos[2].set(pServer->getPos(pPrism, 2));

    pMatrix->mult(mPos[0], mPos[0]);
    pMatrix->mult(mPos[1], mPos[1]);
    pMatrix->mult(mPos[2], mPos[2]);
}
HitInfo::HitInfo() : mParentTriangle(), _60(0.0f), mHitPos(0, 0, 0), _70(0, 0, 0), _7C(0, 0, 0) {
    _88 = 0;
}
const TVec3f* Triangle::getFaceNormal() const {
    return &mNormals[0];
}
const TVec3f* Triangle::getNormal(int index) const {
    return &mNormals[index];
}
Binder::Binder(MtxPtr pMtx, const TVec3f* pPosition, const TVec3f* pGravity, f32 radius, f32 offsetY, u32 planeNum)
    : mTriangleFilter(), mCollisionPartsFilter(), mExCollisionParts(), _C(pMtx), _10(pPosition), _14(pGravity), mRadius(radius), mOffsetY(offsetY),
      mOffsetVec(), _24(planeNum), mPlaneNum(), mPlane(), mFixReactionVector(0, 0, 0), mGroundInfo(), _C8(), mWallInfo(), _158(), mRoofInfo(),
      _1E8() {
    if (_24 == 0) {
        mPlane = nullptr;
    } else {
        mPlane = new HitInfo[_24];
    }

    clear();
    _1EC._0 = true;
    _1EC._1 = true;
    _1EC._2 = false;
    _1EC._3 = false;
    _1EC._4 = false;
    _1EC._5 = false;
}
void Binder::clear() {
    mPlaneNum = 0;
    _C8 = -99999.0f;
    _158 = -99999.0f;
    _1E8 = -99999.0f;
    mFixReactionVector.zero();
}
void Binder::obtainMomentFixReaction(HitInfo* pPlanes, u32 capacity, TVec3f* pReaction, u32 start) {
    TVec3f positive(0, 0, 0);
    TVec3f negative(0, 0, 0);
    for (u32 i = start; i < mPlaneNum; i++) {
        HitInfo& plane = pPlanes[i];
        TVec3f normal(*plane.mParentTriangle.getNormal(0));
        f32 x = normal.x * plane._60;
        if (positive.x < x) {
            positive.x = x;
        } else if (x < negative.x) {
            negative.x = x;
        }

        f32 y = normal.y * plane._60;
        if (positive.y < y) {
            positive.y = y;
        } else if (y < negative.y) {
            negative.y = y;
        }

        f32 z = normal.z * plane._60;
        if (positive.z < z) {
            positive.z = z;
        } else if (z < negative.z) {
            negative.z = z;
        }

        if (_1EC._1 && !MR::isNearZero(plane._7C)) {
            x = plane._7C.x;
            if (positive.x < x) {
                positive.x = x;
            } else if (x < negative.x) {
                negative.x = x;
            }

            y = plane._7C.y;
            if (positive.y < y) {
                positive.y = y;
            } else if (y < negative.y) {
                negative.y = y;
            }

            z = plane._7C.z;
            if (positive.z < z) {
                positive.z = z;
            } else if (z < negative.z) {
                negative.z = z;
            }
        }
    }

    pReaction->set(positive);
    pReaction->add(negative);
}
void Binder::storeContactPlane(HitInfo* pPlanes, u32 capacity) {
    for (u32 i = 0; i < mPlaneNum; i++) {
        const TVec3f& normal = *pPlanes[i].mParentTriangle.getNormal(0);
        if (MR::isFloorPolygon(normal, *_14)) {
            if (_C8 < pPlanes[i]._60) {
                mGroundInfo = pPlanes[i];
                _C8 = pPlanes[i]._60;
            }
        } else if (MR::isWallPolygon(normal, *_14)) {
            if (_158 < pPlanes[i]._60) {
                mWallInfo = pPlanes[i];
                _158 = pPlanes[i]._60;
            }
        } else {
            if (_1E8 < pPlanes[i]._60) {
                mRoofInfo = pPlanes[i];
                _1E8 = pPlanes[i]._60;
            }
        }
    }
}
namespace MR {
bool isSameMtx(MtxPtr pMtxA, MtxPtr pMtxB) {
        f32* pA = (f32*)pMtxA;
        f32* pB = (f32*)pMtxB;
        for (int i = 0; i < 12; i++) {
            if (*pA != *pB) {
                return false;
            }

            pA++;
            pB++;
        }

        return true;
    }
}
