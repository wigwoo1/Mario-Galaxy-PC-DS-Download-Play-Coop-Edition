#include "native_font_records.hpp"
#include <cassert>
#include <fstream>
#include <iterator>
#include <cstdio>
int main(int argc,char**argv){assert(argc==2);std::ifstream file(argv[1],std::ios::binary);assert(file);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});auto reference=decodeResourceFont(BinaryView(bytes));
    NativeFontRecords records(bytes);nw4r::ut::detail::ResFontBase font;font.SetResourceBuffer(&records,&records.info);
    bytes.clear();bytes.shrink_to_fit();
    assert(font.GetWidth()==reference.width && font.GetHeight()==reference.height);
    for(unsigned code=0;code<65536;++code){assert(font.GetGlyphIndex(uint16_t(code))==reference.glyphIndex(uint16_t(code)));}
    for(unsigned code=32;code<127;++code){nw4r::ut::Glyph glyph{};font.GetGlyph(&glyph,uint16_t(code));auto index=reference.glyphIndex(uint16_t(code));auto cell=reference.glyphCell(index);auto widths=reference.glyphWidths(index);
        assert(glyph.cellX==cell[1] && glyph.cellY==cell[2] && glyph.widths.charWidth==widths.advance);
        assert(glyph.pTexture==records.glyph.sheetImage+cell[0]*records.glyph.sheetSize);
    }
    assert(font.RemoveResourceBuffer()==&records);std::puts("PASS owned Wii-to-native font records: 65536 character lookups, ASCII glyph metrics/sheets and source-buffer release");
}
