#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
J3DAnmTexPattern::J3DAnmTexPattern() : mTextureIndex(), mAnmTable(), field_0x14(), mUpdateMaterialNum(), mUpdateMaterialID() {
}

void J3DAnmTexPattern::getTexNo(u16 index, u16* pTexNo) const {
    u32 maxFrame = mAnmTable[index].mMaxFrame;

    if (mFrame < 0.0f) {
        *pTexNo = mTextureIndex[mAnmTable[index].mOffset];
    } else if (mFrame >= (u16)maxFrame) {
        *pTexNo = mTextureIndex[mAnmTable[index].mOffset + ((u16)maxFrame - 1)];
    } else {
        *pTexNo = mTextureIndex[mAnmTable[index].mOffset + (int)mFrame];
    }
}


