#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
void J3DAnmVisibilityFull::getVisibility(u16 index, u8* visibility) const {
    int maxFrame = mAnmTable[index].mMaxFrame;
    int frame = static_cast< int >(mFrame + 0.5f);
    if (frame < 0) {
        *visibility = mVisibility[mAnmTable[index].mOffset];
    } else if (frame >= maxFrame) {
        *visibility = mVisibility[maxFrame + mAnmTable[index].mOffset - 1];
    } else {
        *visibility = mVisibility[mAnmTable[index].mOffset + frame];
    }
}


