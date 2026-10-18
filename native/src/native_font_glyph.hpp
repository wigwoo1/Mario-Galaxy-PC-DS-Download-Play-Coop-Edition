#pragma once
#include "resource_font.hpp"
struct NativeFontGlyphRegion {uint32_t sheet,x,y,width,height;NativeFontWidths metrics;};
inline NativeFontGlyphRegion nativeFontGlyphRegion(const NativeResourceFont& font,uint16_t character){auto index=font.glyphIndex(character);auto cell=font.glyphCell(index);auto metrics=font.glyphWidths(index);const auto& texture=font.sheets.at(cell[0]);assetCheck(metrics.width<=font.cellWidth && cell[1]+metrics.width<=texture.width && cell[2]+font.cellHeight<=texture.height,"Font glyph outside native sheet");return {cell[0],cell[1],cell[2],metrics.width,font.cellHeight,metrics};}
