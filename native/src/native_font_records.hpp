#pragma once
#include "resource_font.hpp"
#ifdef GetCharWidth
#undef GetCharWidth
#endif
#include <nw4r/ut/ResFontBase.h>
#include <memory>
#include <cstring>
// Stable owner of host-layout records and original encoded GX sheets.
class NativeFontRecords {
    using Width=nw4r::ut::FontWidth;
    using Map=nw4r::ut::FontCodeMap;
    std::unique_ptr<unsigned char[]> widthStorage;
    std::vector<std::unique_ptr<unsigned char[]>> mapStorage;
    std::vector<uint8_t> image;
public:
    nw4r::ut::FontInformation info{};
    nw4r::ut::FontTextureGlyph glyph{};
    NativeFontRecords(const NativeFontRecords&)=delete;
    NativeFontRecords& operator=(const NativeFontRecords&)=delete;
    NativeFontRecords(NativeFontRecords&&)=delete;
    explicit NativeFontRecords(const std::vector<uint8_t>& bytes){
        auto decoded=decodeResourceFont(BinaryView(bytes));BinaryView data(bytes);
        size_t cursor=data.u16(12),body=0;
        for(unsigned i=0;i<data.u16(14);++i){if(data.tag(cursor)=="TGLP"){body=cursor+8;break;}cursor+=data.u32(cursor+4);}
        assetCheck(body!=0,"Missing font glyph block");
        glyph.cellWidth=decoded.cellWidth;glyph.cellHeight=decoded.cellHeight;glyph.baselinePos=decoded.baseline;
        glyph.maxCharWidth=data.u8(body+3);glyph.sheetSize=data.u32(body+4);glyph.sheetNum=data.u16(body+8);
        glyph.sheetFormat=data.u16(body+10);glyph.sheetRow=decoded.columns;glyph.sheetLine=decoded.rows;
        glyph.sheetWidth=data.u16(body+16);glyph.sheetHeight=data.u16(body+18);
        auto offset=data.u32(body+20);auto size=size_t(glyph.sheetSize)*glyph.sheetNum;
        extent(offset,size,bytes.size());image.assign(bytes.begin()+offset,bytes.begin()+offset+size);glyph.sheetImage=image.data();
        info.fontType=1;info.linefeed=decoded.linefeed;info.alterCharIndex=decoded.alternate;
        info.defaultWidth={decoded.defaultWidths.left,decoded.defaultWidths.width,decoded.defaultWidths.advance};
        info.encoding=1;info.height=decoded.height;info.width=decoded.width;info.ascent=decoded.ascent;info.pGlyph=&glyph;
        const size_t capacity=size_t(glyph.sheetNum)*glyph.sheetRow*glyph.sheetLine;
        widthStorage=std::make_unique<unsigned char[]>(sizeof(Width)+capacity*sizeof(nw4r::ut::CharWidths));
        auto widths=new(widthStorage.get()) Width{};widths->indexBegin=0;widths->indexEnd=uint16_t(capacity-1);
        for(size_t i=0;i<capacity;++i){auto w=decoded.glyphWidths(uint16_t(i));widths->widthTable[i]={w.left,w.width,w.advance};}info.pWidth=widths;
        Map* previous=nullptr;
        for(const auto& source:decoded.maps){
            auto storage=std::make_unique<unsigned char[]>(sizeof(Map)+(size_t(source.end)-source.begin+1)*sizeof(uint16_t));
            auto map=new(storage.get()) Map{};map->ccodeBegin=source.begin;map->ccodeEnd=source.end;map->mappingMethod=1;
            for(unsigned code=source.begin;code<=source.end;++code){auto found=source.values.find(uint16_t(code));map->mapInfo[code-source.begin]=found==source.values.end()?0xffff:found->second;}
            if(previous)previous->pNext=map;else info.pMap=map;previous=map;mapStorage.push_back(std::move(storage));
        }
    }
};

