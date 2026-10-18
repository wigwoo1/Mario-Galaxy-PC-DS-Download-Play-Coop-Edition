#include <Native/GameFonts.hpp>
#include <Game/System/GameSystemFontHolder.hpp>
#include <nw4r/ut/ResFont.h>
GameSystemFontHolder::GameSystemFontHolder() : _0(), _4(), mEmbeddedMessageFont(), mMessageFont(), mPictureFont(), mMenuFont(), mNumberFont(), mCinemaFont() {
}

nw4r::ut::Font* GameSystemFontHolder::getMessageFont() const {
    if (mEmbeddedMessageFont != nullptr) {
        return mEmbeddedMessageFont;
    }

    return mMessageFont;
}

void GameSystemFontHolder::createFontFromFile() { nativeLoadGameFonts(*this); }
void GameSystemFontHolder::createFontFromEmbeddedData() { nativeLoadEmbeddedGameFonts(*this); }
