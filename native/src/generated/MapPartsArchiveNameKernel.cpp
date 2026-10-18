#include <cstdio>
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
namespace MR {
    void getMapPartsObjectName(char* pBuffer, u32 size, const JMapInfoIter& rIter) {
        const char* pName = "";
        getObjectName(&pName, rIter);
        s32 shapeId = -1;
        getJMapInfoShapeIdWithInit(rIter, &shapeId);
        getMapPartsObjectName(pBuffer, size, pName, shapeId);
    }

    void getMapPartsObjectNameIfExistShapeID(char* pBuffer, u32 size, const JMapInfoIter& rIter) {
        const char* pName = "";
        getObjectName(&pName, rIter);
        s32 shapeId = -1;
        getJMapInfoShapeIdWithInit(rIter, &shapeId);
        if (shapeId >= 0) {
            getMapPartsObjectName(pBuffer, size, pName, shapeId);
        } else {
            snprintf(pBuffer, size, "%s", pName);
        }
    }

    void getMapPartsObjectName(char* pBuffer, u32 size, const char* pName, s32 shapeId) {
        snprintf(pBuffer, size, "%s%02d", pName, shapeId);
    }


}

