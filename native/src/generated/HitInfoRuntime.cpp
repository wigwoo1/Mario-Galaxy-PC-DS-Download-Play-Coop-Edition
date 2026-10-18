#include "Game/Map/HitInfo.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/KCollision.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>





const char* Triangle::getHostName() const {
    return mParts->getHostName();
}

s32 Triangle::getHostPlacementZoneID() const {
    return mParts->getPlacementZoneID();
}



bool Triangle::isValid() const {
    return mIdx != 0xFFFFFFFF;
}





const TVec3f* Triangle::getEdgeNormal(int index) const {
    return &mNormals[index + 1];
}

const TVec3f* Triangle::getPos(int index) const {
    return &mPos[index];
}

const TVec3f* Triangle::calcAndGetNormal(int index) {
    KCollisionServer* server = mParts->mServer;
    KC_PrismData* prism = server->getPrismData(mIdx);

    MtxPtr matrix = reinterpret_cast< MtxPtr >(&mParts->mBaseMatrix);

    switch (index) {
    case 0: {
        mNormals[0].set(*server->getFaceNormal(prism));
        PSMTXMultVecSR(matrix, &mNormals[0], &mNormals[0]);
        MR::normalize(&mNormals[0]);

        return &mNormals[0];
    } break;
    case 1: {
        mNormals[1].set(*server->getEdgeNormal1(prism));
        PSMTXMultVecSR(matrix, &mNormals[1], &mNormals[1]);
        MR::normalize(&mNormals[1]);

        return &mNormals[1];
    } break;
    case 2: {
        mNormals[2].set(*server->getEdgeNormal2(prism));
        PSMTXMultVecSR(matrix, &mNormals[2], &mNormals[2]);
        MR::normalize(&mNormals[2]);

        return &mNormals[2];
    } break;
    case 3: {
        mNormals[3].set(*server->getEdgeNormal3(prism));
        PSMTXMultVecSR(matrix, &mNormals[3], &mNormals[3]);
        MR::normalize(&mNormals[3]);

        return &mNormals[3];
    } break;
    }

    return &mNormals[index];
}

const TVec3f* Triangle::calcAndGetEdgeNormal(int index) {
    KCollisionServer* server = mParts->mServer;
    KC_PrismData* prism = server->getPrismData(mIdx);

    MtxPtr matrix = reinterpret_cast< MtxPtr >(&mParts->mBaseMatrix);

    switch (index) {
    case 0: {
        mNormals[1].set(*server->getEdgeNormal1(prism));
        PSMTXMultVecSR(matrix, &mNormals[1], &mNormals[1]);
        MR::normalize(&mNormals[1]);

        return &mNormals[1];
    } break;
    case 1: {
        mNormals[2].set(*server->getEdgeNormal2(prism));
        PSMTXMultVecSR(matrix, &mNormals[2], &mNormals[2]);
        MR::normalize(&mNormals[2]);

        return &mNormals[2];
    } break;
    case 2: {
        mNormals[3].set(*server->getEdgeNormal3(prism));
        PSMTXMultVecSR(matrix, &mNormals[3], &mNormals[3]);
        MR::normalize(&mNormals[3]);

        return &mNormals[3];
    } break;
    }

    return &mNormals[index + 1];
}

const TVec3f* Triangle::calcAndGetPos(int index) {
    KCollisionServer* server = mParts->mServer;
    KC_PrismData* prism = server->getPrismData(mIdx);

    TVec3f* pos = &mPos[index];

    pos->set(server->getPos(prism, index));

    mParts->mBaseMatrix.mult(*pos, *pos);

    return pos;
}



JMapInfoIter Triangle::getAttributes() const {
    if (mIdx == -1) {
        return JMapInfoIter();
    }

    return mParts->mServer->getAttributes(mIdx);
}

TPos3f* Triangle::getBaseMtx() const {
    return &mParts->mBaseMatrix;
}

TPos3f* Triangle::getBaseInvMtx() const {
    return &mParts->mInvBaseMatrix;
}

TPos3f* Triangle::getPrevBaseMtx() const {
    return &mParts->mPrevBaseMatrix;
}



bool HitInfo::isCollisionAtFace() const {
    return _88 == 1;
}

bool HitInfo::isCollisionAtEdge() const {
    return _88 == 2 || _88 == 3 || _88 == 4;
}

bool HitInfo::isCollisionAtCorner() const {
    return _88 == 5 || _88 == 6 || _88 == 7;
}
