#include <JSystem/JUtility/JUTTexture.hpp>
void JUTTexture::capture(int width, int height, GXTexFmt format, bool mipmap, GXBool clear) {
    if (mFlag & 0x1) {
        if (mipmap) {
            GXSetTexCopySrc(width, height, mTIMG->mWidth * 2, mTIMG->mHeight * 2);
        } else {
            GXSetTexCopySrc(width, height, mTIMG->mWidth, mTIMG->mHeight);
        }

        GXSetTexCopyDst(mTIMG->mWidth, mTIMG->mHeight, format, mipmap);
        GXCopyTex(mImage, clear);
        GXPixModeSync();
    }
}

