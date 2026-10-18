#include <JSystem/J3DGraphLoader/J3DMaterialFactory.hpp>
u16 J3DMaterialFactory::countUniqueMaterials() {
    u16 count = 0;
    s32 id = -1;
    for (u16 i = 0; i < mMaterialNum; i++) {
        if (id < mpMaterialID[i]) {
            id = mpMaterialID[i];
            count++;
        }
    }

    return count;
}
u32 J3DMaterialFactory::countTexGens(int i_idx) const {
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mTexGenNumIdx != 0xff) {
        return mpTexGenNum[mtl_init_data->mTexGenNumIdx];
    }

    return 0;
}
u32 J3DMaterialFactory::countStages(int i_idx) const {
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    u32 count1 = 0;
    u32 count2 = 0;
    if (mtl_init_data->mTevStageNumIdx != 0xff) {
        count2 = mpTevStageNum[mtl_init_data->mTevStageNumIdx];
    }

    for (int i = 0; i < 8; i++) {
        if (mtl_init_data->mTexNoIdx[i] != 0xffff) {
            count1++;
        }
    }

    if (count2 != count1 && count1 != 0) {
        u32 count3;
        if (count2 > count1) {
            count3 = count2;
        } else {
            count3 = count1;
        }

        return count3;
    }

    return count2;
}
const u8 J3DMaterialFactory::newColorChanNum(int i_idx) const {
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mColorChanNumIdx != 0xff) {
        return mpColorChanNum[mtl_init_data->mColorChanNumIdx];
    } else {
        return 0;
    }
}
u32 J3DMaterialFactory::newTexGenNum(int i_idx) const {
    u32 r30 = 0;
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mTexGenNumIdx != 0xff) {
        return mpTexGenNum[mtl_init_data->mTexGenNumIdx];
    } else {
        return 0;
    }
}
u8 J3DMaterialFactory::newCullMode(int i_idx) const {
    u32 r30 = 0;
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mCullModeIdx != 0xff) {
        return mpCullMode[mtl_init_data->mCullModeIdx];
    } else {
        return 0xff;
    }
}
u16 J3DMaterialFactory::newTexNo(int i_idx, int i_no) const {
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mTexNoIdx[i_no] != 0xffff) {
        return mpTexNo[mtl_init_data->mTexNoIdx[i_no]];
    } else {
        return 0xffff;
    }
}
const u8 J3DMaterialFactory::newTevStageNum(int i_idx) const {
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mTevStageNumIdx != 0xff) {
        return mpTevStageNum[mtl_init_data->mTevStageNumIdx];
    } else {
        return 0xff;
    }
}
const u8 J3DMaterialFactory::newZCompLoc(int i_idx) const {
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mZCompLocIdx != 0xff) {
        return mpZCompLoc[mtl_init_data->mZCompLocIdx];
    } else {
        return 0;
    }
}
const u8 J3DMaterialFactory::newDither(int i_idx) const {
    J3DMaterialInitData* mtl_init_data = &mpMaterialInitData[mpMaterialID[i_idx]];
    if (mtl_init_data->mDitherIdx != 0xff) {
        return mpDither[mtl_init_data->mDitherIdx];
    } else {
        return 1;
    }
}
