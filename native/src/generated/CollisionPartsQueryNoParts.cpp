#include <Game/Map/CollisionParts.hpp>
#include <Game/Map/HitInfo.hpp>
#include <Game/Map/KCollision.hpp>
#include <Game/Util/MathUtil.hpp>
#include <Game/Util/TriangleFilter.hpp>
#include <Game/LiveActor/Binder.hpp>
#include <Game/Util/MapUtil.hpp>
#include <Game/Map/CollisionCategorizedKeeper.hpp>
#include <Game/Util/MtxUtil.hpp>
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
