#include <JSystem/J3DGraphAnimator/J3DMaterialAttach.hpp>
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
void J3DMaterialTable::clear() {
    mMaterialNum = 0;
    mUniqueMatNum = 0;
    mMaterialNodePointer = NULL;
    mMaterialName = NULL;
    field_0x10 = 0;
    mTexture = NULL;
    mTextureName = NULL;
    field_0x1c = 0;
}

J3DMaterialTable::J3DMaterialTable() {
    mMaterialNum = 0;
    mUniqueMatNum = 0;
    mMaterialNodePointer = NULL;
    mMaterialName = NULL;
    field_0x10 = 0;
    mTexture = NULL;
    mTextureName = NULL;
    field_0x1c = 0;
}

J3DMaterialTable::~J3DMaterialTable() {
}

void J3DAnmTexPattern::searchUpdateMaterialID(J3DMaterialTable* pMatTable) {
    for (u16 i = 0; i < mUpdateMaterialNum; i++) {
        s32 index = pMatTable->getMaterialName()->getIndex(mUpdateMaterialName.getName(i));
        if (index != -1) {
            mUpdateMaterialID[i] = index;
        } else {
            mUpdateMaterialID[i] = -1;
        }
    }
}


