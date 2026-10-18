#include <cassert>
#include <cstdio>
#include <cstring>
#include <nw4r/ut/ResFontBase.h>
int main(){
    using namespace nw4r::ut;
    detail::ResFontBase font;assert(font.IsManaging(nullptr));
    FontTextureGlyph glyph{};glyph.cellWidth=16;glyph.cellHeight=20;glyph.baselinePos=15;glyph.maxCharWidth=17;
    glyph.sheetRow=4;glyph.sheetLine=4;glyph.sheetNum=1;glyph.sheetWidth=128;glyph.sheetHeight=128;
    FontInformation info{};info.width=16;info.height=20;info.ascent=15;info.linefeed=22;info.alterCharIndex=3;info.defaultWidth={1,12,14};info.pGlyph=&glyph;info.encoding=1;
    alignas(FontCodeMap) unsigned char mapStorage[64]={};auto map=reinterpret_cast<FontCodeMap*>(mapStorage);
    map->ccodeBegin=65;map->ccodeEnd=67;map->mappingMethod=0;map->mapInfo[0]=4;info.pMap=map;
    int resource=0;font.SetResourceBuffer(&resource,&info);
    assert(font.GetWidth()==16 && font.GetHeight()==20 && font.GetAscent()==15 && font.GetDescent()==5);
    assert(font.GetLineFeed()==22 && font.GetCellWidth()==16 && font.GetCellHeight()==20);
    assert(font.FindGlyphIndex(65)==4 && font.FindGlyphIndex(67)==6 && font.FindGlyphIndex(64)==0xffff);
    assert(font.GetGlyphIndex(64)==3 && font.HasGlyph(65) && !font.HasGlyph(64));
    assert(font.GetCharWidth(65)==14);font.SetLineFeed(24);assert(font.GetLineFeed()==24);
    assert(font.SetAlternateChar(66) && font.GetGlyphIndex(64)==5);
    assert(font.RemoveResourceBuffer()==&resource && font.IsManaging(nullptr) && !font.mFontInfo);
    std::puts("PASS original resource-font base native records: metrics, direct map, fallback, mutation and detach");
}
