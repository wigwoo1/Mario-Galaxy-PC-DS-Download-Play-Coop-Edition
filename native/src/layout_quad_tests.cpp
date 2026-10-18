#include "native_font_records.hpp"
#include "message_text.hpp"
#include "native_message_tag_access.hpp"
#include <Game/Screen/MessageTagSkipTagProcessor.hpp>

#include <nw4r/ut/TextWriterBase.h>
#include "native_font_draw_stream.hpp"

#include <nw4r/ut/ResFont.h>
#include <Native/FontResource.hpp>
#include "gx_command_texture.hpp"
#include <nw4r/ut/CharWriter.h>
#include <nw4r/lyt/common.h>
#include <nw4r/math/types.h>
#include <nw4r/ut/Color.h>
#include <revolution/gd/GDBase.h>
#include <Native/GxUiVertices.hpp>
#include <cassert>
#include <cstdio>
#include "native_ui_color_renderer.hpp"
#include "native_disc_archive_loader.hpp"
#include "native_font_glyph.hpp"
// Static cache storage; constructor/destructor, colors, printing and glyph loading are original.

namespace nw4r::ut {
CharWriter::LoadingTexture CharWriter::mLoadingTexture;
}
int main(int argc,char** argv){assert(argc==2);uint8_t bytes[4096];GDLObj list;GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);nw4r::ut::CharWriter::SetupVertexFormat();Native::GxCommandState originalVertexSetup;originalVertexSetup.consume(bytes,GDGetCurrOffset());assert(originalVertexSetup.cp(0x50)==((1u<<9)|(1u<<13)));assert(originalVertexSetup.cp(0x60)==1);assert(originalVertexSetup.xf(0x1008)==17);assert((originalVertexSetup.cp(0x70)&511u)==9);assert(((originalVertexSetup.cp(0x70)>>13)&15u)==11);assert(((originalVertexSetup.cp(0x70)>>21)&511u)==(1u|(2u<<1)|(15u<<4)));GDSetCurrOffset(0);std::puts("PASS original CharWriter vertex setup through native CP/VAT/XF command state");nw4r::math::VEC2 uv[8][4];for(unsigned t=0;t<8;++t)for(unsigned i=0;i<4;++i)uv[t][i]=nw4r::math::VEC2(float(t+i),float(t-i));nw4r::ut::Color colors[4]={0x11223344,0x55667788,0x99aabbcc,0xddeeff00};nw4r::math::VEC2 origin(10,20);nw4r::lyt::Size size(30,40);
for(unsigned colored=0;colored<2;++colored)for(unsigned textures=0;textures<=8;++textures){GDSetCurrOffset(0);nw4r::lyt::detail::DrawQuad(origin,size,textures,uv,colored?colors:nullptr);auto count=GDGetCurrOffset();auto vertices=Native::decodeUiQuad(bytes,count,colored,textures);const unsigned order[]={0,1,3,2};const float xs[]={10,40,40,10},ys[]={20,20,60,60};for(unsigned i=0;i<4;++i){assert(vertices[i].position[0]==xs[i] && vertices[i].position[1]==ys[i]);if(colored){u32 color=colors[order[i]];for(unsigned c=0;c<4;++c)assert(vertices[i].color[c]==uint8_t(color>>(24-8*c)));}else for(auto c:vertices[i].color)assert(c==255);for(unsigned t=0;t<textures;++t)assert(vertices[i].uv[t][0]==uv[t][order[i]].x && vertices[i].uv[t][1]==uv[t][order[i]].y);}bool truncated=false;try{Native::decodeUiQuad(bytes,count-1,colored,textures);}catch(const std::length_error&){truncated=true;}assert(truncated);}
GDSetCurrOffset(0);nw4r::math::VEC2 pixelOrigin(1,1);nw4r::lyt::Size pixelSize(6,6);nw4r::ut::Color solid[4]={0x33669980,0x33669980,0x33669980,0x33669980};nw4r::lyt::detail::DrawQuad(pixelOrigin,pixelSize,0,nullptr,solid);auto quad=Native::decodeUiQuad(bytes,GDGetCurrOffset(),true,0);
for(bool software:{false,true}){NativeUiColorRenderer renderer(software);auto image=renderer.render(quad,8,8);for(unsigned y=0;y<8;++y)for(unsigned x=0;x<8;++x){auto pixel=image.pixels[y*8+x];if(x>=1 && x<7 && y>=1 && y<7)assert(pixel.r==0x33 && pixel.g==0x66 && pixel.b==0x99 && pixel.a==0x80);else assert(pixel.r==0 && pixel.g==0 && pixel.b==0 && pixel.a==0);}std::puts(software?"PASS original quad GPU pixels: WARP":"PASS original quad GPU pixels: hardware");}
TextureImage texture{2,2,6,{{200,0,0,255},{0,200,0,255},{0,0,200,255},{200,200,200,255}}};texture.minFilter=texture.magFilter=0;texture.wrapS=texture.wrapT=0;nw4r::math::VEC2 textureUv[1][4]={{{0,0},{1,0},{0,1},{1,1}}};nw4r::ut::Color tint[4]={0x808080ff,0x808080ff,0x808080ff,0x808080ff};GDSetCurrOffset(0);nw4r::lyt::detail::DrawQuad(pixelOrigin,pixelSize,1,textureUv,tint);auto texturedQuad=Native::decodeUiQuad(bytes,GDGetCurrOffset(),true,1);
for(bool software:{false,true}){NativeUiColorRenderer renderer(software);auto image=renderer.render(texturedQuad,8,8,&texture);for(unsigned y=1;y<7;++y)for(unsigned x=1;x<7;++x){auto expected=texture.pixels[(y>=4?2:0)+(x>=4?1:0)],pixel=image.pixels[y*8+x];assert(std::abs(int(pixel.r)-int(std::round(expected.r*128.f/255)))<=1 && std::abs(int(pixel.g)-int(std::round(expected.g*128.f/255)))<=1 && std::abs(int(pixel.b)-int(std::round(expected.b*128.f/255)))<=1 && pixel.a==255);}std::puts(software?"PASS original textured quad UV/modulation pixels: WARP":"PASS original textured quad UV/modulation pixels: hardware");}
NativeDiscArchiveLoader disc(std::filesystem::u8path(argv[1]));auto fontArchive=disc.mount("/LayoutData/Font.arc");auto resource=fontArchive->get("/menufont64.brfnt");assert(resource);std::vector<uint8_t> fontBytes(resource,resource+fontArchive->sizeOf(resource));auto font=decodeResourceFont(BinaryView(fontBytes));fontArchive.reset();disc.unmount("/LayoutData/Font.arc");
for(bool software:{false,true}){NativeUiColorRenderer renderer(software);for(uint16_t character:{uint16_t('A'),uint16_t('?')}){auto glyph=nativeFontGlyphRegion(font,character);assert(glyph.width && glyph.height);auto sheet=font.sheets[glyph.sheet];sheet.minFilter=sheet.magFilter=0;float left=float(glyph.x)/sheet.width,top=float(glyph.y)/sheet.height,right=float(glyph.x+glyph.width)/sheet.width,bottom=float(glyph.y+glyph.height)/sheet.height;nw4r::math::VEC2 glyphUv[1][4]={{{left,top},{right,top},{left,bottom},{right,bottom}}};nw4r::ut::Color white[4]={0xffffffff,0xffffffff,0xffffffff,0xffffffff};GDSetCurrOffset(0);nw4r::lyt::detail::DrawQuad(nw4r::math::VEC2(1,1),nw4r::lyt::Size(float(glyph.width),float(glyph.height)),1,glyphUv,white);auto vertices=Native::decodeUiQuad(bytes,GDGetCurrOffset(),true,1);auto image=renderer.render(vertices,glyph.width+2,glyph.height+2,&sheet);for(unsigned y=0;y<glyph.height;++y)for(unsigned x=0;x<glyph.width;++x){auto expected=sheet.pixels[(glyph.y+y)*sheet.width+glyph.x+x],pixel=image.pixels[(y+1)*image.width+x+1];assert(pixel.r==expected.r && pixel.g==expected.g && pixel.b==expected.b && pixel.a==expected.a);}}
std::puts(software?"PASS actual game font glyph GPU pixels: WARP":"PASS actual game font glyph GPU pixels: hardware");}
{
 nw4r::ut::CharWriter writer;writer.EnableLinearFilter(false,false); nw4r::ut::Glyph glyph{};auto region=nativeFontGlyphRegion(font,'A');auto& sheet=font.sheets[region.sheet];
 nw4r::ut::ResFont runtimeFont;assert(nativeRegisterFontResource(fontBytes.data(),fontBytes.size()));assert(runtimeFont.SetResource(fontBytes.data()));nativeUnregisterFontResource(fontBytes.data());runtimeFont.GetGlyph(&glyph,'A');writer.SetFont(runtimeFont);
 writer.SetTextColor(0x12345680);writer.SetAlpha(128);GDSetCurrOffset(0);writer.PrintGlyph(1,1,0,glyph);assert(GDGetCurrOffset()==108);auto tinted=Native::decodeFontQuad(bytes+25,83);for(auto& vertex:tinted){assert(vertex.color[0]==0x12 && vertex.color[1]==0x34 && vertex.color[2]==0x56 && vertex.color[3]==64);}
 writer.SetAlpha(255);writer.SetTextColor(0xffffffff);writer.ResetTextureCache();
 GDSetCurrOffset(0);writer.PrintGlyph(1,1,0,glyph);assert(GDGetCurrOffset()==108);Native::GxCommandState boundState;boundState.consume(bytes,25);auto boundTexture=decodeCommandTexture(boundState,0,Native::gpuResources());Native::GxCommandState streamState;auto firstDraws=decodeNativeFontDraws(streamState,bytes,108,Native::gpuResources());assert(firstDraws.size()==1 && firstDraws[0].texture->pixels.size()==boundTexture.pixels.size());assert(firstDraws[0].intensityFont==(unsigned(glyph.texFormat)<=1));assert(boundTexture.width==sheet.width && boundTexture.height==sheet.height);
 GDSetCurrOffset(0);nw4r::ut::CharWriter::SetupVertexFormat();writer.ResetTextureCache();writer.PrintGlyph(1,1,0,glyph);Native::GxCommandState configured;auto configuredDraws=decodeNativeFontDraws(configured,bytes,GDGetCurrOffset(),Native::gpuResources());assert(configured.cpConfigured() && configuredDraws.size()==1);GDSetCurrOffset(0);GXSetVtxDesc(GX_VA_POS,GX_INDEX16);writer.PrintGlyph(1,1,0,glyph);bool wrongFormat=false;try{decodeNativeFontDraws(configured,bytes,GDGetCurrOffset(),Native::gpuResources());}catch(const std::exception&){wrongFormat=true;}assert(wrongFormat && ((configured.cp(0x50)>>9)&3)==1);GDSetCurrOffset(0);writer.ResetTextureCache();writer.PrintGlyph(1,1,0,glyph);
 std::puts("PASS original vertex setup plus glyph stream and mismatched-format rollback");
 GDSetCurrOffset(0);writer.SetupGX();writer.PrintGlyph(1,1,0,glyph);Native::GxCommandState fullFontSetup;auto fullSetupDraws=decodeNativeFontDraws(fullFontSetup,bytes,GDGetCurrOffset(),Native::gpuResources());assert(fullSetupDraws.size()==1 && fullSetupDraws[0].alphaBlend);assert(fullSetupDraws[0].intensityFont==(unsigned(glyph.texFormat)<=1));assert(fullFontSetup.tevStageCount()==1 && fullFontSetup.depthTexture().operation==0);GDSetCurrOffset(0);writer.ResetTextureCache();writer.PrintGlyph(1,1,0,glyph);
 for(bool software:{false,true}){NativeUiColorRenderer renderer(software);auto& draw=fullSetupDraws[0];auto target=renderer.renderBatch({{draw.vertices,draw.texture.get(),draw.alphaBlend,draw.intensityFont}},region.width+2,region.height+2);for(unsigned y=0;y<region.height;++y)for(unsigned x=0;x<region.width;++x){auto sample=sheet.pixels[(region.y+y)*sheet.width+region.x+x];auto actual=target.pixels[(y+1)*target.width+x+1];unsigned red=draw.intensityFont?255:sample.r,green=draw.intensityFont?255:sample.g,blue=draw.intensityFont?255:sample.b;auto close=[](unsigned a,unsigned b){return a>b?a-b<=1:b-a<=1;};assert(close(actual.r,(red*sample.a+127)/255) && close(actual.g,(green*sample.a+127)/255) && close(actual.b,(blue*sample.a+127)/255) && close(actual.a,(unsigned(sample.a)*sample.a+127)/255));}}
 std::puts("PASS original CharWriter SetupGX plus actual glyph native command stream and GPU pixels");
 auto vertices=Native::decodeFontQuad(bytes+25,83);assert(vertices[0].position[0]==1 && vertices[2].position[0]==1+region.width && vertices[2].position[1]==1+region.height);
 bool rejected=false;try{Native::decodeFontQuad(bytes+25,82);}catch(const std::exception&){rejected=true;}assert(rejected);
 for(bool software:{false,true}){auto texture=boundTexture;texture.minFilter=texture.magFilter=0;NativeUiColorRenderer renderer(software);auto image=renderer.render(vertices,region.width+2,region.height+2,&texture);for(unsigned y=0;y<region.height;++y)for(unsigned x=0;x<region.width;++x){auto expected=sheet.pixels[(region.y+y)*sheet.width+region.x+x],actual=image.pixels[(y+1)*image.width+x+1];assert(expected.r==actual.r && expected.g==actual.g && expected.b==actual.b && expected.a==actual.a);}}
 GDSetCurrOffset(0);writer.PrintGlyph(1,1,0,glyph);assert(GDGetCurrOffset()==83);
 unsigned char unrelated[32]={};Native::gpuResources().add(unrelated,sizeof(unrelated));
 GDSetCurrOffset(0);writer.PrintGlyph(1,1,0,glyph);assert(GDGetCurrOffset()==108);Native::gpuResources().remove(unrelated);
 GDSetCurrOffset(0);writer.PrintGlyph(1,1,0,glyph);assert(GDGetCurrOffset()==108);
 writer.SetCursor(1,1,0);GDSetCurrOffset(0);auto advance=writer.Print('A');assert(advance==runtimeFont.GetCharWidth('A'));assert(writer.GetCursorX()==1+advance && writer.GetCursorY()==1);assert(GDGetCurrOffset()==83);auto printed=Native::decodeFontQuad(bytes,83);assert(printed[0].position[0]==1+glyph.widths.left && printed[0].position[1]==1);
 auto cachedDraws=decodeNativeFontDraws(streamState,bytes,83,Native::gpuResources());assert(cachedDraws.size()==1 && cachedDraws[0].vertices[0].position==printed[0].position);
 uint8_t malformed[]={0x61,0x80,0,0,0,0xff};auto savedMode=streamState.bp(0x80);bool malformedRejected=false;try{decodeNativeFontDraws(streamState,malformed,sizeof(malformed),Native::gpuResources());}catch(const std::exception&){malformedRejected=true;}assert(malformedRejected && streamState.bp(0x80)==savedMode);
 bool shortDraw=false;try{decodeNativeFontDraws(streamState,bytes,82,Native::gpuResources());}catch(const std::exception&){shortDraw=true;}assert(shortDraw);
 writer.SetCursor(8,1,0);GDSetCurrOffset(0);writer.Print('A');writer.MoveCursorX(2);writer.Print('A');auto pair=decodeNativeFontDraws(streamState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(pair.size()==2 && pair[0].texture==pair[1].texture);
 unsigned targetWidth=unsigned(2*runtimeFont.GetCharWidth('A')+region.width+8),targetHeight=region.height+2;
 for(bool software:{false,true}){NativeUiColorRenderer renderer(software);std::vector<NativeUiColorRenderer::Draw> batch;for(auto& draw:pair)batch.push_back({draw.vertices,draw.texture.get(),draw.alphaBlend,draw.intensityFont});auto combined=renderer.renderBatch(batch,targetWidth,targetHeight);for(unsigned y=0;y<targetHeight;++y)for(unsigned x=0;x<targetWidth;++x){Pixel expected{0,0,0,0};for(auto& draw:pair){auto xOrigin=unsigned(draw.vertices[0].position[0]);if(y>=1 && y<1+region.height && x>=xOrigin && x<xOrigin+region.width){expected=sheet.pixels[(region.y+y-1)*sheet.width+region.x+x-xOrigin];if(draw.intensityFont)expected.r=expected.g=expected.b=255;}}auto actual=combined.pixels[y*targetWidth+x];assert(expected.r==actual.r && expected.g==actual.g && expected.b==actual.b && expected.a==actual.a);}}
 std::puts("PASS original two-glyph stream GPU batch: hardware/WARP");
 GDSetCurrOffset(0);GDWriteBPCmd(0x410004a1);writer.Print('A');auto blended=decodeNativeFontDraws(streamState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(blended.size()==1 && blended[0].alphaBlend);
 std::puts("PASS native font command stream: bound/cached draws and malformed-stream rollback");
 runtimeFont.InitReaderFunc(nw4r::ut::FONT_ENCODING_CP1252);nw4r::ut::TextWriterBase<char> textWriter;textWriter.SetFont(runtimeFont);textWriter.EnableLinearFilter(false,false);textWriter.SetCursor(8,8,0);textWriter.SetCharSpace(2);textWriter.ResetTextureCache();
 GDSetCurrOffset(0);GDWriteBPCmd(0x41000000);textWriter.Print("AA\nA",4);Native::GxCommandState textState;auto textDraws=decodeNativeFontDraws(textState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(textDraws.size()==3);assert(textDraws[1].vertices[0].position[0]-textDraws[0].vertices[0].position[0]==runtimeFont.GetCharWidth('A')+2);assert(textDraws[2].vertices[0].position[0]==textDraws[0].vertices[0].position[0]);assert(textDraws[2].vertices[0].position[1]-textDraws[0].vertices[0].position[1]==runtimeFont.GetLineFeed());
 for(bool software:{false,true}){NativeUiColorRenderer renderer(software);std::vector<NativeUiColorRenderer::Draw> batch;for(auto& draw:textDraws)batch.push_back({draw.vertices,draw.texture.get(),draw.alphaBlend,draw.intensityFont});auto output=renderer.renderBatch(batch,160,256);for(unsigned y=0;y<256;++y)for(unsigned x=0;x<160;++x){Pixel expected{0,0,0,0};for(auto& draw:textDraws){int left=int(draw.vertices[0].position[0]),top=int(draw.vertices[0].position[1]);if(int(x)>=left && int(x)<left+int(region.width) && int(y)>=top && int(y)<top+int(region.height)){expected=sheet.pixels[(region.y+y-top)*sheet.width+region.x+x-left];if(draw.intensityFont)expected.r=expected.g=expected.b=255;}}auto actual=output.pixels[y*160+x];assert(expected.r==actual.r && expected.g==actual.g && expected.b==actual.b && expected.a==actual.a);}}
 runtimeFont.InitReaderFunc(nw4r::ut::FONT_ENCODING_UTF16);nw4r::ut::TextWriterBase<wchar_t> wideWriter;wideWriter.SetFont(runtimeFont);wideWriter.EnableLinearFilter(false,false);wideWriter.SetCursor(8,8,0);wideWriter.SetCharSpace(2);wideWriter.ResetTextureCache();GDSetCurrOffset(0);GDWriteBPCmd(0x41000000);wideWriter.Print(L"AA\nA",4);Native::GxCommandState wideState;auto wideDraws=decodeNativeFontDraws(wideState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(wideDraws.size()==textDraws.size());for(size_t i=0;i<wideDraws.size();++i)for(unsigned j=0;j<4;++j){assert(wideDraws[i].vertices[j].position==textDraws[i].vertices[j].position);assert(wideDraws[i].vertices[j].uv==textDraws[i].vertices[j].uv);}
 for(bool software:{false,true}){NativeUiColorRenderer renderer(software);std::vector<NativeUiColorRenderer::Draw> narrowBatch,wideBatch;for(auto& draw:textDraws)narrowBatch.push_back({draw.vertices,draw.texture.get(),draw.alphaBlend,draw.intensityFont});for(auto& draw:wideDraws)wideBatch.push_back({draw.vertices,draw.texture.get(),draw.alphaBlend,draw.intensityFont});auto expected=renderer.renderBatch(narrowBatch,160,256),actual=renderer.renderBatch(wideBatch,160,256);assert(expected.pixels.size()==actual.pixels.size());for(size_t i=0;i<actual.pixels.size();++i)assert(expected.pixels[i].r==actual.pixels[i].r && expected.pixels[i].g==actual.pixels[i].g && expected.pixels[i].b==actual.pixels[i].b && expected.pixels[i].a==actual.pixels[i].a);}
 wideWriter.SetCursor(32,32,0);wideWriter.ResetTextureCache();GDSetCurrOffset(0);wideWriter.Print(L"\u00e9",1);auto unicodeDraws=decodeNativeFontDraws(wideState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(unicodeDraws.size()==1);auto unicodeRegion=nativeFontGlyphRegion(font,0xe9);auto& unicodeSheet=font.sheets[unicodeRegion.sheet];assert(unicodeDraws[0].vertices[0].uv[0][0]==float(unsigned(unicodeRegion.x*32768/unicodeSheet.width))/32768.f);assert(wideWriter.GetCursorX()==32+runtimeFont.GetCharWidth(0xe9));
 std::puts("PASS original UTF16 non-ASCII glyph selection and advance");
 {NativeBmgText::Token a;a.code=L'A';NativeBmgText::Token tag;tag.code=0x1a;tag.tag={6,0,0,0};NativeTaggedMessage text({a,tag,a});NativeMessageTagScope scope(text);MessageTagSkipTagProcessor skip;assert(skip.NativeCustomProcessor()==nullptr);auto* previousProcessor=&wideWriter.GetTagProcessor();wideWriter.SetTagProcessor(&skip);wideWriter.SetCursor(8,8,0);wideWriter.ResetTextureCache();GDSetCurrOffset(0);wideWriter.Print(text.data(),int(text.size()));auto skipped=decodeNativeFontDraws(wideState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(skipped.size()==2);assert(wideWriter.GetCursorX()==8+2*runtimeFont.GetCharWidth(L'A')+2);wideWriter.SetTagProcessor(previousProcessor);}
 std::puts("PASS original UTF16 tag-skip printing without custom-processor cast");
 auto messageArchive=disc.mount("/UsEnglish/MessageData/Message.arc");auto messageResource=messageArchive->get("/message.bmg");assert(messageResource);NativeBmgText retailMessages(std::vector<uint8_t>(messageResource,messageResource+messageArchive->sizeOf(messageResource)));std::wstring retailText;
 for(size_t index=0;index<retailMessages.size();++index){auto tokens=retailMessages.tokens(index);if(tokens.empty() || tokens.size()>8)continue;bool plain=true;for(auto& token:tokens)plain&=token.code>=33 && token.code<=126;if(plain){retailText=retailMessages.plainHostText(index);break;}}
 {std::unique_ptr<NativeTaggedMessage> taggedRetail;std::wstring visible;for(size_t index=0;index<retailMessages.size();++index){auto tokens=retailMessages.tokens(index);bool hasTag=false,accepted=true;std::wstring text;for(const auto& token:tokens){if(token.code==0x1a)hasTag=true;else if(token.code>=33 && token.code<=126)text.push_back(wchar_t(token.code));else accepted=false;}if(hasTag && accepted && !text.empty() && text.size()<=16){taggedRetail=std::make_unique<NativeTaggedMessage>(std::move(tokens));visible=std::move(text);break;}}assert(taggedRetail);MessageTagSkipTagProcessor skip;auto* previous=&wideWriter.GetTagProcessor();NativeMessageTagScope scope(*taggedRetail);wideWriter.SetTagProcessor(&skip);wideWriter.SetCursor(16,16,0);wideWriter.ResetTextureCache();GDSetCurrOffset(0);wideWriter.Print(taggedRetail->data(),int(taggedRetail->size()));Native::GxCommandState taggedState;auto taggedDraws=decodeNativeFontDraws(taggedState,bytes,GDGetCurrOffset(),Native::gpuResources());wideWriter.SetTagProcessor(previous);wideWriter.SetCursor(16,16,0);wideWriter.ResetTextureCache();GDSetCurrOffset(0);wideWriter.Print(visible.c_str(),int(visible.size()));Native::GxCommandState referenceState;auto reference=decodeNativeFontDraws(referenceState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(taggedDraws.size()==reference.size());for(size_t i=0;i<reference.size();++i)for(unsigned j=0;j<4;++j){assert(taggedDraws[i].vertices[j].position==reference[i].vertices[j].position);assert(taggedDraws[i].vertices[j].uv==reference[i].vertices[j].uv);}for(bool software:{false,true}){NativeUiColorRenderer renderer(software);std::vector<NativeUiColorRenderer::Draw> taggedBatch,referenceBatch;for(auto& d:taggedDraws)taggedBatch.push_back({d.vertices,d.texture.get(),d.alphaBlend,d.intensityFont});for(auto& d:reference)referenceBatch.push_back({d.vertices,d.texture.get(),d.alphaBlend,d.intensityFont});auto actual=renderer.renderBatch(taggedBatch,1024,128),expected=renderer.renderBatch(referenceBatch,1024,128);assert(actual.pixels.size()==expected.pixels.size());for(size_t i=0;i<actual.pixels.size();++i)assert(actual.pixels[i].r==expected.pixels[i].r && actual.pixels[i].g==expected.pixels[i].g && actual.pixels[i].b==expected.pixels[i].b && actual.pixels[i].a==expected.pixels[i].a);}}
 std::puts("PASS actual retail tagged message through original tag-skip printing and GPU pixels: hardware/WARP");
 assert(!retailText.empty());messageArchive.reset();disc.unmount("/UsEnglish/MessageData/Message.arc");wideWriter.SetCursor(32,32,0);wideWriter.ResetTextureCache();GDSetCurrOffset(0);wideWriter.SetupGX();wideWriter.Print(retailText.c_str(),int(retailText.size()));auto retailDraws=decodeNativeFontDraws(wideState,bytes,GDGetCurrOffset(),Native::gpuResources());assert(retailDraws.size()==retailText.size());float expectedCursor=32;for(auto character:retailText)expectedCursor+=runtimeFont.GetCharWidth(character)+2;expectedCursor-=2;assert(wideWriter.GetCursorX()==expectedCursor);
 for(bool software:{false,true}){NativeUiColorRenderer renderer(software);std::vector<NativeUiColorRenderer::Draw> batch;for(auto& draw:retailDraws)batch.push_back({draw.vertices,draw.texture.get(),draw.alphaBlend,draw.intensityFont});auto output=renderer.renderBatch(batch,512,128);for(unsigned y=0;y<128;++y)for(unsigned x=0;x<512;++x){Pixel expected{0,0,0,0};for(size_t index=0;index<retailDraws.size();++index){auto& draw=retailDraws[index];auto cell=nativeFontGlyphRegion(font,uint16_t(retailText[index]));auto& atlas=font.sheets[cell.sheet];int left=int(draw.vertices[0].position[0]),top=int(draw.vertices[0].position[1]);if(int(x)>=left && int(x)<left+int(cell.width) && int(y)>=top && int(y)<top+int(cell.height)){auto source=atlas.pixels[(cell.y+y-top)*atlas.width+cell.x+x-left];if(draw.intensityFont)source.r=source.g=source.b=255;if(draw.alphaBlend){auto blend=[&](unsigned src,unsigned dst){return uint8_t((src*source.a+dst*(255-source.a)+127)/255);};expected={blend(source.r,expected.r),blend(source.g,expected.g),blend(source.b,expected.b),blend(source.a,expected.a)};}else expected=source;}}auto actual=output.pixels[y*512+x];auto close=[](unsigned a,unsigned b){return a>b?a-b<=2:b-a<=2;};assert(close(expected.r,actual.r) && close(expected.g,actual.g) && close(expected.b,actual.b) && close(expected.a,actual.a));}}
 std::puts("PASS actual WBFS plain message through original SetupGX/UTF16 printing and blended GPU pixels: hardware/WARP");
 std::puts("PASS original UTF16 multiline print and native GPU pixels: hardware/WARP");
 std::puts("PASS multiline native text printing to GPU pixels: hardware/WARP");
 std::puts("PASS original CharWriter lifecycle, color/alpha and character advance");
 std::puts("PASS font texture cache hit and registration/removal invalidation");
 std::puts("PASS original CharWriter glyph vertices to GPU: hardware/WARP");
}
for(bool software:{false,true}){NativeUiColorRenderer renderer(software);auto background=quad,foreground=quad;for(auto& v:background)v.color={0,200,0,255};for(auto& v:foreground)v.color={255,0,0,128};auto image=renderer.renderBatch({{background,nullptr,false},{foreground,nullptr,true}},8,8);for(unsigned y=1;y<7;++y)for(unsigned x=1;x<7;++x){auto pixel=image.pixels[y*8+x];assert(std::abs(int(pixel.r)-128)<=1 && std::abs(int(pixel.g)-100)<=1 && pixel.b==0 && std::abs(int(pixel.a)-191)<=1);}}
std::puts("PASS source-alpha GPU batch blending: hardware/WARP");
for(bool software:{false,true}){NativeUiColorRenderer renderer(software);auto colored=quad;for(auto& vertex:colored)vertex.color={100,150,200,128};TextureImage intensity{1,1,0,{{64,64,64,64}}};intensity.minFilter=intensity.magFilter=0;intensity.wrapS=intensity.wrapT=0;auto image=renderer.renderBatch({{colored,&intensity,false,true}},8,8);for(unsigned y=1;y<7;++y)for(unsigned x=1;x<7;++x){auto pixel=image.pixels[y*8+x];assert(pixel.r==100 && pixel.g==150 && pixel.b==200 && std::abs(int(pixel.a)-32)<=1);}}
std::puts("PASS original intensity-font RGB/alpha shader equation: hardware/WARP");
std::puts("PASS original layout quad emission and native vertex decoding: 18 color/texture formats");}



































