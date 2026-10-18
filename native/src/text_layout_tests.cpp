#include <Native/FontResource.hpp>
#include "native_font_records.hpp"
#include <nw4r/ut/ResFont.h>
#include <nw4r/ut/TextWriterBase.h>
#include <cassert>
#include <fstream>
#include <iterator>
#include <cstdio>
#include <vector>
struct Emitted {uint16_t code;float x,y;};
static std::vector<Emitted> emitted;
namespace nw4r::ut {
CharWriter::LoadingTexture CharWriter::mLoadingTexture;
CharWriter::CharWriter():mAlpha(Color::ALPHA_MAX),mIsWidthFixed(false),mFixedWidth(0),mFont(nullptr){ResetColorMapping();SetGradationMode(GRADMODE_NONE);SetTextColor(Color::WHITE);SetScale(1,1);SetCursor(0,0,0);}
CharWriter::~CharWriter(){}
void CharWriter::UpdateVertexColor(){}
f32 CharWriter::GetFontWidth()const{return mFont->GetWidth()*mScale.x;}
f32 CharWriter::GetFontHeight()const{return mFont->GetHeight()*mScale.y;}
f32 CharWriter::GetFontAscent()const{return mFont->GetAscent()*mScale.y;}
// Recording sink isolates layout/control flow from GX texture/GPU dependencies.
f32 CharWriter::Print(CharCode code){emitted.push_back({code,mCursorPos.x,mCursorPos.y});float width=mIsWidthFixed?mFixedWidth:mFont->GetCharWidth(code)*mScale.x;mCursorPos.x+=width;return width;}
}
int main(int argc,char** argv){assert(argc==2);std::ifstream file(argv[1],std::ios::binary);assert(file);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});nw4r::ut::ResFont font;assert(nativeRegisterFontResource(bytes.data(),bytes.size()));assert(font.SetResource(bytes.data()));nativeUnregisterFontResource(bytes.data());
// Real font is UTF16, while the narrow writer requires a narrow character reader.
font.InitReaderFunc(nw4r::ut::FONT_ENCODING_CP1252);
nw4r::ut::TextWriterBase<char> writer;writer.SetFont(font);writer.SetCursor(10,20);writer.SetCharSpace(2);const char text[]="AB\nC";float width=writer.Print(text,4);assert(emitted.size()==3);assert(emitted[0].code=='A' && emitted[1].code=='B' && emitted[2].code=='C');float a=font.GetCharWidth('A'),b=font.GetCharWidth('B'),c=font.GetCharWidth('C');assert(emitted[0].x==10 && emitted[1].x==10+a+2 && emitted[2].x==10);assert(emitted[2].y-emitted[0].y==font.GetLineFeed());assert(width==c);assert(writer.GetCursorX()==10+c);
emitted.clear();writer.SetCursor(0,0);writer.SetWidthLimit(a+2);writer.Print("AB",2);assert(emitted.size()==2 && emitted[1].x==0 && emitted[1].y-emitted[0].y==font.GetLineFeed());
std::puts("PASS native narrow print layout: actual font advances, spacing, newline and cursor");}



