#include <Native/GameFonts.hpp>
#include <Native/HeapPlacement.hpp>
#include "Game/System/GameSystemFontHolder.hpp"
#include "Game/System/FileLoader.hpp"
#include "Game/System/Language.hpp"
#include "Game/Util/FileUtil.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <cstdio>
#include <nw4r/ut/ResFont.h>

GameSystemFontHolder::GameSystemFontHolder() : _0(), _4(), mEmbeddedMessageFont(), mMessageFont(), mPictureFont(), mMenuFont(), mNumberFont(), mCinemaFont() {
}

nw4r::ut::Font* GameSystemFontHolder::getMessageFont() const {
    if (mEmbeddedMessageFont != nullptr) {
        return mEmbeddedMessageFont;
    }

    return mMessageFont;
}

void GameSystemFontHolder::createFontFromEmbeddedData() { nativeLoadEmbeddedGameFonts(*this); }
namespace {
    inline nw4r::ut::ResFont* createFont(JKRArchive* pArchive, const char* pName) {
        nw4r::ut::ResFont* pFont = new nw4r::ut::ResFont();
        pFont->SetResource(pArchive->getResource(pName));
        return pFont;
    }
}  // namespace

void GameSystemFontHolder::createFontFromFile() { nativeLoadGameFonts(*this); }
