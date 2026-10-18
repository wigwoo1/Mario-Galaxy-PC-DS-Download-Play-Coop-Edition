#include "native_disc_archive_loader.hpp"
#include "native_archive_fonts.hpp"
#include "native_disc_font_backend.hpp"
#include <cassert>
#include <Native/GameFonts.hpp>
#include <Game/System/GameSystemFontHolder.hpp>
#include <cstdio>
#include <Game/System/ErrorArchive.hpp>
int main(int argc,char**argv){assert(argc==2);NativeDiscArchiveLoader disc(std::filesystem::u8path(argv[1]));
    assert(be32(cErrorArchive+4)<=sizeof(cErrorArchive));
    OwnedArchiveResources embedded(std::vector<uint8_t>(cErrorArchive,cErrorArchive+be32(cErrorArchive+4)));
    auto nestedBytes=embedded.get("/usenglish/layoutdata/embeddedfont.arc");assert(nestedBytes);
    OwnedArchiveResources nested(std::vector<uint8_t>(nestedBytes,nestedBytes+embedded.sizeOf(nestedBytes)));
    auto message=nested.get("/messagefont26.brfnt");assert(message);
    nw4r::ut::ResFont errorFont;assert(nativeRegisterFontResource(message,nested.sizeOf(message)));
    assert(errorFont.SetResource(const_cast<uint8_t*>(message)));nativeUnregisterFontResource(message);
    assert(errorFont.GetWidth()>0 && errorFont.GetHeight()>0);
    nw4r::ut::Glyph errorGlyph{};errorFont.GetGlyph(&errorGlyph,'?');assert(errorGlyph.pTexture);
    errorFont.RemoveResource();
    auto archive=disc.mount("/LayoutData/Font.arc");NativeArchiveFonts fonts(*archive);
    assert(disc.unmount("/LayoutData/Font.arc"));archive.reset();
    for(auto& font:fonts.fonts){assert(font->GetWidth()>0 && font->GetHeight()>0);nw4r::ut::Glyph glyph{};font->GetGlyph(&glyph,'?');assert(glyph.pTexture && glyph.texWidth && glyph.texHeight);}
    GameSystemFontHolder holder;NativeDiscFontBackend backend(disc);
    bool rejected=false;try{holder.createFontFromFile();}catch(const std::runtime_error&){rejected=true;}assert(rejected);
    nativeBindGameFontBackend(&backend);holder.createFontFromEmbeddedData();assert(holder.getMessageFont()==holder.mEmbeddedMessageFont && holder.mEmbeddedMessageFont);holder.createFontFromFile();assert(!holder.mEmbeddedMessageFont);assert(holder.getMessageFont()==holder.mMessageFont);
    auto* miiFont=nativeLoadMiiNameFont();assert(miiFont&&miiFont!=holder.mMessageFont&&nativeLoadMiiNameFont()==miiFont);
    assert(miiFont->GetHeight()==holder.mMessageFont->GetHeight());
    for(wchar_t letter:L"Mario Luigi 0123456789?")if(letter)assert(miiFont->HasGlyph(letter));
    assert(holder.mPictureFont && holder.mMenuFont && holder.mNumberFont && holder.mCinemaFont);
    assert(disc.unmount("/LayoutData/Font.arc"));assert(holder.getMessageFont()->GetWidth()>0);assert(miiFont->HasGlyph('M'));
    rejected=false;try{holder.createFontFromFile();}catch(const std::runtime_error&){rejected=true;}assert(rejected);
    assert(backend.release(holder));assert(!backend.release(holder));assert(!holder.getMessageFont() && !holder.mCinemaFont);
    holder.createFontFromEmbeddedData();assert(holder.getMessageFont());holder.createFontFromFile();assert(holder.getMessageFont() && !holder.mEmbeddedMessageFont);
    assert(backend.release(holder));nativeBindGameFontBackend(nullptr);
    {NativeDiscFontBackend temporary(disc);nativeBindGameFontBackend(&temporary);holder.createFontFromEmbeddedData();assert(holder.getMessageFont());holder.createFontFromFile();assert(holder.getMessageFont() && !holder.mEmbeddedMessageFont);}
    assert(!holder.getMessageFont() && !holder.mPictureFont && !holder.mMenuFont && !holder.mNumberFont && !holder.mCinemaFont);
    rejected=false;try{holder.createFontFromFile();}catch(const std::runtime_error&){rejected=true;}assert(rejected);
    {NativeDiscFontBackend missing(disc,"NotALanguage");nativeBindGameFontBackend(&missing);rejected=false;try{holder.createFontFromEmbeddedData();}catch(const std::runtime_error&){rejected=true;}assert(rejected && !holder.getMessageFont());}
    {NativeDiscFontBackend errorOnly(disc);nativeBindGameFontBackend(&errorOnly);holder.createFontFromEmbeddedData();assert(holder.mEmbeddedMessageFont);}
    assert(!holder.getMessageFont());
    std::puts("PASS five campaign fonts loaded from WBFS Font.arc through bounded ResFont; glyphs retained after archive release");
    std::puts("PASS USA-disc Mii-name font: independent native font, expected glyphs, reuse and archive-release lifetime");
}












