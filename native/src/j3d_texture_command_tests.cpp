#include <revolution/gx/GXTransform.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXLighting.h>
#include <revolution/gx/GXTexture.h>
#include <JSystem/J3DGraphBase/J3DGD.hpp>
#include <JSystem/J3DGraphBase/J3DFifo.hpp>
#include <JSystem/J3DGraphBase/J3DTevs.hpp>
#include <revolution/mtx.h>
#include <revolution/os/OSCache.h>
#include <revolution/gd/GDTev.h>
#include <revolution/gd/GDIndirect.h>
#include <Native/GpuResources.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/TevStage.hpp>
#include "gx_command_texture.hpp"
#include "preview.hpp"
#include <nw4r/lyt/texMap.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <stdexcept>

static u32 word(const u8* b) {
    assert(b[0] == 0x61);
    return u32(b[1]) << 24 | u32(b[2]) << 16 | u32(b[3]) << 8 | b[4];
}
int main(int argc,char** argv) {
 std::setvbuf(stdout,nullptr,_IONBF,0);
 std::set_terminate([]{void* frames[16]{};auto count=RtlCaptureStackBackTrace(0,16,frames,nullptr);auto base=reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));std::fprintf(stderr,"Unhandled texture test exception; executable RVAs:");for(unsigned i=0;i<count;++i)std::fprintf(stderr," %llX",static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(frames[i])-base));std::fprintf(stderr,"\n");std::abort();});
 {u8 bytes[64]{};GDLObj list;const unsigned hardware[]={0,1,2,3,4,4,4,5};
  for(unsigned f=0;f<8;++f)for(unsigned z=0;z<4;++z){
   GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);Native::GxCommandState state;
   GDWriteBPCmd(0x43abcdef);GDWriteBPCmd(0x005abcde);GDWriteBPCmd(0x42123456);state.consume(bytes,15);GDSetCurrOffset(0);
   GXSetPixelFmt(GXPixelFmt(f),GXZFmt16(z));assert(GDGetCurrOffset()==(hardware[f]==4?30:20));state.consume(bytes,GDGetCurrOffset());
   assert(state.bp(0x43)==((0xabcdef&~63u)|hardware[f]|(z<<3)));
   assert(state.bp(0)==((0x5abcde&~0x200u)|(f==2?0x200u:0)));
   assert(state.bp(0x42)==(hardware[f]==4?((0x123456&~0x600u)|((f-4)<<9)):0x123456));
  }
  GDSetCurrOffset(0);bool rejected=false;try{GXSetPixelFmt(GXPixelFmt(8),GX_ZC_LINEAR);}catch(const std::invalid_argument&){rejected=true;}assert(rejected && GDGetCurrOffset()==0);
 }
 {u8 bytes[32]{};GDLObj list;GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);
  GDWriteBPCmd(0xfe000001);GXInvalidateTexAll();assert(GDGetCurrOffset()==25);
  assert(word(bytes+5)==0xfeffffff && word(bytes+10)==0x66001000 && word(bytes+15)==0x66001100 && word(bytes+20)==0xfeffffff);
  Native::GxCommandState state;state.consume(bytes,25);assert(state.bp(0x66)==0x1100);
  GDSetCurrOffset(0);GDWriteBPCmd(0x225abcde);state.consume(bytes,5);assert(state.bp(0x22)==0x5abcde);
 }
 {u8 bytes[16]{};GDLObj list;Native::GxCommandState state;
  for(unsigned size=0;size<256;++size)for(unsigned offset=0;offset<6;++offset){
   GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);GDWriteBPCmd(0x225abcde);state.consume(bytes,5);GDSetCurrOffset(0);GXSetLineWidth(u8(size),GXTexOffset(offset));assert(GDGetCurrOffset()==10);state.consume(bytes,10);assert(state.bp(0x22)==((0x5abcde&~0x0700ffu)|size|(offset<<16)));
   GDSetCurrOffset(0);GXSetPointSize(u8(size),GXTexOffset(offset));state.consume(bytes,10);assert(state.bp(0x22)==((0x5abcde&~0x3fffFFu)|size|(offset<<16)|(size<<8)|(offset<<19)));
  }
 }
 {float matrix[3][4]={{1,2,3,99},{4,5,6,88},{7,8,9,77}};u8 bytes[64]{};GDLObj list;
  for(unsigned slot=0;slot<10;++slot){GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);GXLoadNrmMtxImm(matrix,slot*3);assert(GDGetCurrOffset()==41&&bytes[0]==0x10);u32 hdr=u32(bytes[1])<<24|u32(bytes[2])<<16|u32(bytes[3])<<8|bytes[4];assert(hdr==(8u<<16|(0x400+slot*9)));for(unsigned i=0;i<9;++i){unsigned offset=5+i*4;u32 bits=u32(bytes[offset])<<24|u32(bytes[offset+1])<<16|u32(bytes[offset+2])<<8|bytes[offset+3];float value;std::memcpy(&value,&bits,4);assert(value==float(i+1));}}
  GDSetCurrOffset(0);bool rejected=false;try{GXLoadNrmMtxImm(matrix,1);}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 }
    { // Exercise the original inline loader, not just SDK packing macros.
      for(unsigned index=0;index<16;++index){
        J3DTevStage stage;
        auto* packed=reinterpret_cast<u8*>(&stage);
        for(unsigned byte=0;byte<8;++byte)packed[byte]=u8(17+index*11+byte*29);
        stage.setStageNo(index);
        u8 commands[10]{};GDLObj list;GDInitGDLObj(&list,commands,sizeof(commands));GDSetCurrent(&list);
        stage.load(index);assert(GDGetCurrOffset()==10);
        Native::GxCommandState state;state.consume(commands,sizeof(commands));
        for(unsigned command=0;command<2;++command){
          auto* bytes=packed+command*4;
          assert(word(commands+command*5)==(u32(bytes[0])<<24|u32(bytes[1])<<16|u32(bytes[2])<<8|bytes[3]));
          assert(state.bp(0xc0+index*2+command)==(u32(bytes[1])<<16|u32(bytes[2])<<8|bytes[3]));
        }
      }GDSetCurrent(nullptr);
      std::puts("PASS original TEV stage loader: all sixteen stage register pairs and host-independent byte packing");
    }
    {Native::GxCommandState pipeline;u8 bytes[64]{};GDLObj list;GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);
     GDWriteBPCmd(BP_TEV_KSEL(0,1,0,0,0,0,0xf6));GDWriteBPCmd(BP_TEV_KSEL(2,3,0,0,0,0,0xf7));
     GDWriteBPCmd(TEV_COLOR_ENV(8,15,15,15,0,0,1,0,1,0xc0));
     GDWriteBPCmd(TEV_ALPHA_ENV(0,0,4,7,7,7,0,0,1,0,2,0xc1));pipeline.consume(bytes,GDGetCurrOffset());
     Native::TevRegisters registers{{{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}}};
     Native::evaluateTevStage(pipeline,0,registers,{20,30,40,50},{60,70,80,90});
     assert((registers[1]==Native::TevRgba{20,30,40,8}));assert((registers[2]==Native::TevRgba{9,10,11,50}));
     GDSetCurrOffset(0);GDWriteBPCmd(TEV_COLOR_ENV(15,15,15,2,0,0,1,0,0,0xc2));GDWriteBPCmd(TEV_ALPHA_ENV(0,0,7,7,7,2,0,0,1,0,0,0xc3));pipeline.consume(bytes,GDGetCurrOffset());
     Native::evaluateTevStage(pipeline,1,registers,{0,0,0,0},{0,0,0,0});
     assert((registers[0]==Native::TevRgba{20,30,40,50}));
     GDSetCurrOffset(0);GDWriteBPCmd(0x00000400);GDWriteBPCmd(BP_TEV_ORDER(0,0,1,0,0,0,0,0,0x28));pipeline.consume(bytes,GDGetCurrOffset());
     Native::TevFragmentInputs fragment;fragment.textures[0]={20,30,40,50};
     assert((Native::evaluateTevProgram(pipeline,fragment)==Native::TevRgba{20,30,40,50}));
     GDSetCurrOffset(0);GDWriteBPCmd(BP_TEV_ORDER(0,0,0,0,0,0,0,0,0x28));pipeline.consume(bytes,GDGetCurrOffset());
     assert((Native::evaluateTevProgram(pipeline,fragment)==Native::TevRgba{255,255,255,255}));GDSetCurrent(nullptr);
    }
    {uint8_t bytes[32]{};GDLObj list;Native::GxCommandState state;
     const uint8_t seed[]={0x61,0,0x12,0x34,0x56,0x61,0x41,0x65,0x43,0x21};
     state.consume(seed,sizeof(seed));
     for(unsigned mode=0;mode<4;++mode){GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);GXSetCullMode(GXCullMode(mode));assert(GDGetCurrOffset()==10 && word(bytes)==0xfe00c000);state.consume(bytes,10);unsigned hardware=((mode&1)<<1)|((mode&2)>>1);assert(state.bp(0)==((0x123456&~0xc000u)|(hardware<<14)) && state.bp(0xfe)==0xffffff);}
     for(unsigned flag:{1u,0u}){GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);GXSetDither(GXBool(flag));assert(GDGetCurrOffset()==10 && word(bytes)==0xfe000004);state.consume(bytes,10);assert(state.bp(0x41)==((0x654321&~4u)|(flag<<2)) && state.bp(0xfe)==0xffffff);}
     GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);bool rejected=false;try{GXSetCullMode(GXCullMode(4));}catch(const std::invalid_argument&){rejected=true;}assert(rejected && GDGetCurrOffset()==0);rejected=false;try{GXSetDither(GXBool(2));}catch(const std::invalid_argument&){rejected=true;}assert(rejected && GDGetCurrOffset()==0);GDSetCurrent(nullptr);}
    {uint8_t bytes[32]{};GDLObj list;Native::GxCommandState state;const uint8_t seed[]={0x61,0,0x12,0x34,0x56};state.consume(seed,sizeof(seed));for(unsigned value:{1u,0u}){GDInitGDLObj(&list,bytes,sizeof(bytes));GDSetCurrent(&list);GXSetCoPlanar(GXBool(value));assert(GDGetCurrOffset()==10 && bytes[0]==0x61 && bytes[1]==0xfe && bytes[2]==8 && bytes[3]==0 && bytes[4]==0);state.consume(bytes,10);assert(state.bp(0)==((0x123456u&~0x80000u)|(value<<19)) && state.bp(0xfe)==0xffffff);}GDSetCurrent(nullptr);}
    {Native::GxCommandState state; const uint8_t commands[]={0x48,0,0x48};state.consume(commands,sizeof(commands));assert(state.vertexCacheGeneration()==2 && state.bp(0xfe)==0xffffff && !state.cpConfigured());const uint8_t bad[]={0x48,0x61};bool rejected=false;try{state.consume(bad,sizeof(bad));}catch(const std::length_error&){rejected=true;}assert(rejected && state.vertexCacheGeneration()==2);}
    alignas(32) u8 texture[96] = {};
    texture[32] = 0x7b;
    auto& resources = Native::gpuResources();
    const u32 address = resources.add(texture, sizeof texture);
    {
        alignas(32) u8 layoutTexture[64]{};
        const auto layoutAddress=resources.add(layoutTexture,sizeof(layoutTexture));
        GXTexObj original{};GXInitTexObj(&original,layoutTexture,4,4,GX_TF_RGBA8,GX_CLAMP,GX_CLAMP,GX_FALSE);
        nw4r::lyt::TexMap overlay(original);
        GXTexObj rebuilt{};
        try{overlay.Get(&rebuilt);}catch(const std::exception& error){std::fprintf(stderr,"FAIL texture -> NW4R TexMap -> texture round trip: %s\n",error.what());return 1;}
        void* host=nullptr;u16 w,h;GXTexFmt fmt;GXTexWrapMode ws,wt;GXBool mip;
        GXGetTexObjAll(&rebuilt,&host,&w,&h,&fmt,&ws,&wt,&mip);
        assert(host==layoutTexture&&resources.address(host)==layoutAddress&&w==4&&h==4&&fmt==GX_TF_RGBA8);
        layoutTexture[0]=17;resources.refresh(layoutTexture,sizeof(layoutTexture));overlay.Get(&rebuilt);
        assert(resources.resolve(layoutAddress,1)[0]==17);
        resources.remove(layoutTexture);bool stale=false;try{overlay.Get(&rebuilt);}catch(const std::exception&){stale=true;}assert(stale);
        std::puts("PASS texture -> original NW4R TexMap -> texture preserves original host pointer, refresh and stale-resource rejection");
    }
    {GXTexObj roundtrip{};GXInitTexObj(&roundtrip,texture,4,4,GX_TF_RGBA8,GX_REPEAT,GX_MIRROR,GX_FALSE);GXInitTexObjLOD(&roundtrip,GX_LIN_MIP_LIN,GX_LINEAR,2.0f,8.0f,-0.5f,GX_TRUE,GX_FALSE,GX_ANISO_2);GXInitTexObjTlut(&roundtrip,17);assert(GXGetTexObjTlut(&roundtrip)==17);void* image=nullptr;u16 width=0,height=0;GXTexFmt format;GXTexWrapMode wrapS,wrapT;GXBool mipmap;GXGetTexObjAll(&roundtrip,&image,&width,&height,&format,&wrapS,&wrapT,&mipmap);assert(image==texture && width==4 && height==4 && format==GX_TF_RGBA8 && wrapS==GX_REPEAT && wrapT==GX_MIRROR && !mipmap);GXTexFilter minimum,maximum;float low,high,bias;GXBool clamp,edge;GXAnisotropy anisotropy;GXGetTexObjLODAll(&roundtrip,&minimum,&maximum,&low,&high,&bias,&clamp,&edge,&anisotropy);assert(minimum==GX_LIN_MIP_LIN && maximum==GX_LINEAR && low==2 && high==8 && bias==-0.5f && clamp && !edge && anisotropy==GX_ANISO_2);}{for(unsigned fmt=0;fmt<3;++fmt){GXTlutObj palette{};GXInitTlutObj(&palette,texture+32,GXTlutFmt(fmt),16);GXTlutObjInt decoded{};std::memcpy(&decoded,&palette,sizeof(decoded));assert(decoded.numEntries==16 && (decoded.loadTlut0>>24)==0x64 && ((decoded.loadTlut0&0xffffff)<<5)==address+32);}for(unsigned fmt:{8u,9u,10u}){GXTexObj indexed{};GXInitTexObjCI(&indexed,texture,4,4,GXCITexFmt(fmt),GX_CLAMP,GX_CLAMP,GX_FALSE,9);assert(GXGetTexObjTlut(&indexed)==9);}GXTlutObj unchanged{};auto before=unchanged;bool rejected=false;try{GXInitTlutObj(&unchanged,texture+32,GX_TL_RGB565,100);}catch(const std::exception&){rejected=true;}assert(rejected && !std::memcmp(&before,&unchanged,sizeof(before)));}u8 commands[128] = {};
    GDLObj dl;
    GDInitGDLObj(&dl, commands, sizeof commands);
    GDSetCurrent(&dl);
    {alignas(32) u8 paletteBytes[512]{};paletteBytes[0]=0xf8;resources.add(paletteBytes,sizeof(paletteBytes));alignas(32) u8 indexedBytes[64]{};
for(unsigned paletteFormat=0;paletteFormat<3;++paletteFormat){paletteBytes[0]=paletteFormat==1?0xf8:0xff;paletteBytes[1]=paletteFormat==0?0x7f:paletteFormat==1?0:0xff;resources.refresh(paletteBytes,sizeof(paletteBytes));for(unsigned format:{8u,9u,10u}){GDSetCurrOffset(0);GXTlutObj palette{};GXInitTlutObj(&palette,paletteBytes,GXTlutFmt(paletteFormat),256);GXLoadTlut(&palette,0);resources.add(indexedBytes,sizeof(indexedBytes));GXTexObj indexed{};GXInitTexObjCI(&indexed,indexedBytes,4,4,GXCITexFmt(format),GX_CLAMP,GX_CLAMP,GX_FALSE,0);GXLoadTexObj(&indexed,GX_TEXMAP0);Native::GxCommandState indexedState;indexedState.consume(commands,GDGetCurrOffset(),&resources);auto decoded=decodeCommandTexture(indexedState,0,resources);assert(decoded.pixels.size()==16);for(const auto& pixel:decoded.pixels)assert(pixel.r==(paletteFormat==0?127:255) && pixel.g==(paletteFormat==0?127:paletteFormat==1?0:255) && pixel.b==(paletteFormat==0?127:paletteFormat==1?0:255) && pixel.a==255);verifyRendering(decoded,true);verifyRendering(decoded,false);resources.remove(indexedBytes);}}resources.remove(paletteBytes);GDSetCurrOffset(0);} const u8 attrIds[8] = {0x88,0x89,0x8a,0x8b,0xa8,0xa9,0xaa,0xab};
    const u8 pointerIds[8] = {0x94,0x95,0x96,0x97,0xb4,0xb5,0xb6,0xb7};
    for (unsigned i = 0; i < 8; ++i) {
        J3DGDSetTexImgAttr(GXTexMapID(i), 64, 32, GX_TF_RGBA8);
        J3DGDSetTexImgPtr(GXTexMapID(i), texture + 32);
        assert(word(commands+i*10) == (u32(attrIds[i])<<24 | 6u<<20 | 31u<<10 | 63u));
        const u32 pointer = word(commands+i*10+5);
        assert(pointer >> 24 == pointerIds[i]);
        assert(((pointer & 0xffffff) << 5) == address + 32);
        assert(*resources.resolve((pointer & 0xffffff) << 5, 1) == 0x7b);
    }
    assert(GDGetCurrOffset() == 80);
    Native::GxCommandState state;for(unsigned stage=0;stage<4;++stage)for(unsigned scale=0;scale<9;++scale){GDSetCurrOffset(0);GXSetIndTexCoordScale(GXIndTexStageID(stage),GXIndTexScale(scale),GXIndTexScale(8-scale));assert(GDGetCurrOffset()==10);state.consume(commands,10);assert(((state.bp(0x25+stage/2)>>((stage&1)*8))&255)==(scale|((8-scale)<<4)));}for(unsigned matrix:{1u,2u,3u,5u,6u,7u,9u,10u,11u}){const f32 values[2][3]={{0.5f,-0.5f,0.25f},{-0.25f,0.125f,-0.125f}};GDSetCurrOffset(0);GXSetIndTexMtx(GXIndTexMtxID(matrix),values,0);assert(GDGetCurrOffset()==20);unsigned index=(matrix-1)%4;assert((word(commands+5)&0xffffff)==(512u|(1792u<<11)|(1u<<22)));assert((word(commands+5)>>24)==6+index*3);}GDSetCurrOffset(0);
    // Original SDK packing macros exercise every stage and all field widths.
    // These are register decoding checks, not a TEV shader implementation.
    for(unsigned stage=0;stage<16;++stage){
        GDSetCurrOffset(0);
        GDWriteBPCmd(TEV_COLOR_ENV(15-stage,stage,(stage+5)&15,(stage+9)&15,
            stage&3,(stage>>1)&1,stage&1,(stage>>2)&3,(stage>>1)&3,0xc0+stage*2));
        GDWriteBPCmd(TEV_ALPHA_ENV(stage&3,(stage>>2)&3,stage&7,(stage+1)&7,
            (stage+3)&7,(stage+5)&7,(stage+1)&3,stage&1,(stage>>1)&1,
            (stage+2)&3,(stage+3)&3,0xc1+stage*2));
        state.consume(commands,GDGetCurrOffset());
        const auto tev=state.tevStage(stage);
        assert((tev.color.inputs==std::array<unsigned,4>{(stage+9)&15,(stage+5)&15,stage,15-stage}));
        assert(tev.color.bias==(stage&3) && tev.color.scale==((stage>>2)&3));
        assert(tev.color.destination==((stage>>1)&3));
        assert(tev.color.subtract==bool((stage>>1)&1) && tev.color.clamp==bool(stage&1));
        assert((tev.alpha.inputs==std::array<unsigned,4>{(stage+5)&7,(stage+3)&7,(stage+1)&7,stage&7}));
        assert(tev.alpha.bias==((stage+1)&3) && tev.alpha.scale==((stage+2)&3));
        assert(tev.alpha.destination==((stage+3)&3));
        assert(tev.alpha.subtract==bool(stage&1) && tev.alpha.clamp==bool((stage>>1)&1));
        assert(tev.rasterSwap==(stage&3) && tev.textureSwap==((stage>>2)&3));
    }
    for(unsigned pair=0;pair<8;++pair)for(unsigned selector=0;selector<32;++selector){
        GDSetCurrOffset(0);
        GDWriteBPCmd(BP_TEV_KSEL(pair&3,(pair+1)&3,selector,31-selector,
                              (selector+7)&31,(selector+13)&31,0xf6+pair));
        state.consume(commands,GDGetCurrOffset());
        auto even=state.tevKonst(pair*2),odd=state.tevKonst(pair*2+1);
        assert(even.color==selector && even.alpha==31-selector);
        assert(odd.color==((selector+7)&31) && odd.alpha==((selector+13)&31));
    }
    {Native::GxCommandState constants;
     for(unsigned reg=0;reg<4;++reg){GDSetCurrOffset(0);
        GDWriteBPCmd(BP_TEV_COLOR_REG_RA(10+reg,40+reg,1,0xe0+reg*2));
        GDWriteBPCmd(BP_TEV_COLOR_REG_BG(30+reg,20+reg,1,0xe1+reg*2));
        constants.consume(commands,GDGetCurrOffset());}
     const int expectedFractions[]={255,223,191,159,128,96,64,32};
     for(unsigned sel=0;sel<32;++sel){GDSetCurrOffset(0);GDWriteBPCmd(BP_TEV_KSEL(0,0,sel,sel,0,0,0xf6));constants.consume(commands,GDGetCurrOffset());auto value=constants.tevKonstValue(0);
        if(sel<8)assert((value==std::array<int,4>{expectedFractions[sel],expectedFractions[sel],expectedFractions[sel],expectedFractions[sel]}));
        else if(sel<12)assert((value==std::array<int,4>{0,0,0,0}));
        else if(sel<16){unsigned r=sel-12;assert((value==std::array<int,4>{int(10+r),int(20+r),int(30+r),0}));}
        else{int expected=10*((sel-16)/4+1)+(sel&3);assert((value==std::array<int,4>{expected,expected,expected,expected}));}
     }
    }
    bool badKonst=false;try{state.tevKonst(16);}catch(const std::out_of_range&){badKonst=true;}assert(badKonst);
    bool badTevStage=false;
    try{state.tevStage(16);}catch(const std::out_of_range&){badTevStage=true;}
    assert(badTevStage);
    const auto beforeInputs=state.tevStage(15);
    GDSetCurrOffset(0);
    GXSetTevColorIn(GX_TEVSTAGE15,GX_CC_TEXC,GX_CC_RASC,GX_CC_KONST,GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE15,GX_CA_TEXA,GX_CA_RASA,GX_CA_KONST,GX_CA_ZERO);
    state.consume(commands,GDGetCurrOffset());
    const auto afterInputs=state.tevStage(15);
    assert((afterInputs.color.inputs==std::array<unsigned,4>{GX_CC_TEXC,GX_CC_RASC,GX_CC_KONST,GX_CC_ZERO}));
    assert((afterInputs.alpha.inputs==std::array<unsigned,4>{GX_CA_TEXA,GX_CA_RASA,GX_CA_KONST,GX_CA_ZERO}));
    assert(afterInputs.color.bias==beforeInputs.color.bias && afterInputs.color.scale==beforeInputs.color.scale);
    assert(afterInputs.color.destination==beforeInputs.color.destination && afterInputs.color.clamp==beforeInputs.color.clamp);
    assert(afterInputs.color.subtract==beforeInputs.color.subtract);
    assert(afterInputs.alpha.bias==beforeInputs.alpha.bias && afterInputs.alpha.scale==beforeInputs.alpha.scale);
    assert(afterInputs.alpha.destination==beforeInputs.alpha.destination && afterInputs.alpha.clamp==beforeInputs.alpha.clamp);
    assert(afterInputs.alpha.subtract==beforeInputs.alpha.subtract);
    assert(afterInputs.textureSwap==beforeInputs.textureSwap && afterInputs.rasterSwap==beforeInputs.rasterSwap);
    const auto inputOffset=GDGetCurrOffset();
    bool badInput=false;
    try{GXSetTevAlphaIn(GX_TEVSTAGE0,GXTevAlphaArg(8),GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO);}
    catch(const std::invalid_argument&){badInput=true;}
    assert(badInput && GDGetCurrOffset()==inputOffset);
    badInput=false;
    try{GXSetTevColorIn(GXTevStageID(-1),GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO);}
    catch(const std::invalid_argument&){badInput=true;}
    assert(badInput && GDGetCurrOffset()==inputOffset);
    u8 shortInputs[9]={};
    GDLObj shortInputList;
    GDInitGDLObj(&shortInputList,shortInputs,sizeof shortInputs);
    GDSetCurrent(&shortInputList);
    bool inputOverflow=false;
    try{GXSetTevColorIn(GX_TEVSTAGE0,GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO);}
    catch(const std::length_error&){inputOverflow=true;}
    assert(inputOverflow && GDGetCurrOffset()==0);
    GDSetCurrent(&dl);
    for(unsigned operation : {0u,1u,8u,9u,10u,11u,12u,13u,14u,15u}){
        GDSetCurrOffset(0);
        GXSetTevColorOp(GX_TEVSTAGE15,GXTevOp(operation),GX_TB_SUBHALF,GX_CS_DIVIDE_2,GX_TRUE,GX_TEVREG2);
        GXSetTevAlphaOp(GX_TEVSTAGE15,GXTevOp(operation),GX_TB_ADDHALF,GX_CS_SCALE_2,GX_FALSE,GX_TEVREG1);
        state.consume(commands,GDGetCurrOffset());
        const auto current=state.tevStage(15);
        assert(current.color.inputs==afterInputs.color.inputs && current.alpha.inputs==afterInputs.alpha.inputs);
        assert(current.rasterSwap==afterInputs.rasterSwap && current.textureSwap==afterInputs.textureSwap);
        assert(current.color.bias==(operation<=1 ? 2u : 3u));
        assert(current.alpha.bias==(operation<=1 ? 1u : 3u));
        assert(current.color.scale==(operation<=1 ? 3u : (operation>>1)&3));
        assert(current.alpha.scale==(operation<=1 ? 1u : (operation>>1)&3));
        assert(current.color.subtract==bool(operation&1) && current.alpha.subtract==bool(operation&1));
        assert(current.color.clamp && !current.alpha.clamp);
        assert(current.color.destination==3 && current.alpha.destination==2);
    }
    // Different paired orders must survive updates to their neighbor.
    GDSetCurrOffset(0);
    GXSetTevOrder(GX_TEVSTAGE14,GX_TEXCOORD6,GX_TEXMAP5,GX_ALPHA_BUMP);
    GXSetTevOrder(GX_TEVSTAGE15,GX_TEXCOORD2,GXTexMapID(GX_TEXMAP3|GX_TEX_DISABLE),GX_COLOR1A1);
    state.consume(commands,GDGetCurrOffset());
    auto order=state.tevOrder(14);
    assert(order.texture==5 && order.coordinate==6 && order.channel==5 && order.enabled);
    order=state.tevOrder(15);
    assert(order.texture==3 && order.coordinate==2 && order.channel==1 && !order.enabled);
    GDSetCurrOffset(0);
    GXSetTevOrder(GX_TEVSTAGE14,GX_TEXCOORD_NULL,GX_TEXMAP_NULL,GX_COLOR_NULL);
    state.consume(commands,GDGetCurrOffset());
    order=state.tevOrder(14);
    assert(order.texture==0 && order.coordinate==0 && order.channel==7 && !order.enabled);
    order=state.tevOrder(15);
    assert(order.texture==3 && order.coordinate==2 && order.channel==1 && !order.enabled);
    const auto beforeSwap=state.tevStage(15);
    GDSetCurrOffset(0);
    GXSetTevSwapMode(GX_TEVSTAGE15,GX_TEV_SWAP2,GX_TEV_SWAP1);
    GXSetTevKColorSel(GX_TEVSTAGE12,GXTevKColorSel(17));
    GXSetTevKAlphaSel(GX_TEVSTAGE14,GXTevKAlphaSel(23));
    GXSetTevSwapModeTable(GX_TEV_SWAP3,GX_CH_BLUE,GX_CH_ALPHA,GX_CH_RED,GX_CH_GREEN);
    state.consume(commands,GDGetCurrOffset());
    const auto afterSwap=state.tevStage(15);
    assert(afterSwap.rasterSwap==2 && afterSwap.textureSwap==1);
    assert(afterSwap.alpha.inputs==beforeSwap.alpha.inputs && afterSwap.alpha.bias==beforeSwap.alpha.bias);
    assert(afterSwap.alpha.destination==beforeSwap.alpha.destination && afterSwap.alpha.scale==beforeSwap.alpha.scale);
    assert(afterSwap.alpha.clamp==beforeSwap.alpha.clamp && afterSwap.alpha.subtract==beforeSwap.alpha.subtract);
    assert((state.tevSwapTable(3)==std::array<unsigned,4>{2,3,0,1}));
    assert(((state.bp(0xfc)>>4)&31)==17 && ((state.bp(0xfd)>>9)&31)==23);
    for(unsigned table=0;table<4;++table){
        GDSetCurrOffset(0);
        GXSetTevSwapModeTable(GXTevSwapSel(table),GXTevColorChan(table),GXTevColorChan((table+1)&3),
            GXTevColorChan((table+2)&3),GXTevColorChan((table+3)&3));
        state.consume(commands,GDGetCurrOffset());
        assert((state.tevSwapTable(table)==std::array<unsigned,4>{table,(table+1)&3,(table+2)&3,(table+3)&3}));
    }
    const auto swapOffset=GDGetCurrOffset();
    bool badSwap=false;
    try{GXSetTevSwapModeTable(GXTevSwapSel(4),GX_CH_RED,GX_CH_GREEN,GX_CH_BLUE,GX_CH_ALPHA);}
    catch(const std::invalid_argument&){badSwap=true;}
    assert(badSwap && GDGetCurrOffset()==swapOffset);
    for(unsigned compare=0;compare<8;++compare){
        for(unsigned operation=0;operation<4;++operation){
            GDSetCurrOffset(0);
            GXSetAlphaCompare(GXCompare(compare),37,GXAlphaOp(operation),GXCompare(7-compare),219);
            assert(word(commands)==TEV_ALPHAFUNC(37,219,compare,7-compare,operation,0xf3));
            state.consume(commands,GDGetCurrOffset());
            const auto test=state.alphaCompare();
            assert(test.firstReference==37 && test.secondReference==219);
            assert(test.first==compare && test.second==7-compare && test.operation==operation);
        }
    }
    const auto alphaOffset=GDGetCurrOffset();
    bool badAlpha=false;
    try{GXSetAlphaCompare(GXCompare(8),0,GX_AOP_AND,GX_ALWAYS,255);}
    catch(const std::invalid_argument&){badAlpha=true;}
    assert(badAlpha && GDGetCurrOffset()==alphaOffset);
    GDSetCurrOffset(0);
    GDWriteBPCmd(0x0007c25a);
    state.consume(commands,GDGetCurrOffset());
    const auto unrelatedGeneral=state.bp(0)&~0x3c00u;
    for(unsigned count=1;count<=16;++count){
        GDSetCurrOffset(0);
        GXSetNumTevStages(u8(count));
        state.consume(commands,GDGetCurrOffset());
        assert(state.tevStageCount()==count);
        assert((state.bp(0)&~0x3c00u)==unrelatedGeneral);
    }
    for(unsigned invalidCount : {0u,17u,255u}){
        const auto beforeCount=GDGetCurrOffset();
        bool badCount=false;
        try{GXSetNumTevStages(u8(invalidCount));}catch(const std::invalid_argument&){badCount=true;}
        assert(badCount && GDGetCurrOffset()==beforeCount);
    }
    Mtx textureMatrix={{1,2,3,4},{-5,-6,-7,-8},{9.5f,10.5f,11.5f,12.5f}};
    for(unsigned type=0;type<2;++type)for(unsigned id:{3u,30u,33u,57u,60u,64u,67u,121u,125u}){unsigned address=id>=64?0x500+((id-64)<<2):id<<2,count=type==1?8:12;GDSetCurrOffset(0);GDWriteXFCmd(u16(address-1),0x12345678);GDWriteXFCmd(u16(address+count),0x87654321);state.consume(commands,GDGetCurrOffset());GDSetCurrOffset(0);GXLoadTexMtxImm(textureMatrix,id,GXTexMtxType(type));assert(GDGetCurrOffset()==5+4*count);state.consume(commands,GDGetCurrOffset());for(unsigned i=0;i<count;++i){u32 bits;std::memcpy(&bits,&textureMatrix[i/4][i%4],4);assert(state.xf(address+i)==bits);}assert(state.xf(address-1)==0x12345678 && state.xf(address+count)==0x87654321);}
    auto matrixBefore=GDGetCurrOffset();for(unsigned id:{61u,63u,126u,0xffffffffu}){bool bad=false;try{GXLoadTexMtxImm(textureMatrix,id,GX_MTX3x4);}catch(const std::invalid_argument&){bad=true;}assert(bad && GDGetCurrOffset()==matrixBefore);}
    std::puts("PASS texture-matrix XF uploads: primary/post banks, both shapes and bounded indices");
    GDSetCurrOffset(0);GDWriteBPCmd(0x41ab001c);state.consume(commands,GDGetCurrOffset());
    const auto preservedBlend=state.bp(0x41)&~0xffe3u;
    for(unsigned mode=0;mode<4;++mode)for(unsigned source=0;source<8;++source)for(unsigned destination=0;destination<8;++destination)for(unsigned logic=0;logic<16;++logic){GDSetCurrOffset(0);GXSetBlendMode(GXBlendMode(mode),GXBlendFactor(source),GXBlendFactor(destination),GXLogicOp(logic));assert(GDGetCurrOffset()==10);state.consume(commands,GDGetCurrOffset());auto value=state.bp(0x41);assert((value&~0xffe3u)==preservedBlend);assert((value&1)==(mode&1) && ((value>>1)&1)==(mode==2) && ((value>>11)&1)==(mode==3));assert(((value>>5)&7)==destination && ((value>>8)&7)==source && ((value>>12)&15)==logic);}
    for(unsigned enabled=0;enabled<2;++enabled){auto before=state.bp(0x41);GDSetCurrOffset(0);GXSetColorUpdate(GXBool(enabled));state.consume(commands,GDGetCurrOffset());assert((state.bp(0x41)&~8u)==(before&~8u) && ((state.bp(0x41)>>3)&1)==enabled);before=state.bp(0x41);GDSetCurrOffset(0);GXSetAlphaUpdate(GXBool(enabled));state.consume(commands,GDGetCurrOffset());assert((state.bp(0x41)&~16u)==(before&~16u) && ((state.bp(0x41)>>4)&1)==enabled);}
    auto beforeBlend=GDGetCurrOffset();bool badBlend=false;try{GXSetBlendMode(GXBlendMode(4),GX_BL_ONE,GX_BL_ZERO,GX_LO_COPY);}catch(const std::invalid_argument&){badBlend=true;}assert(badBlend && GDGetCurrOffset()==beforeBlend);
    std::puts("PASS 4096 blend configurations and independent color/alpha write masks");
    for(unsigned channel=0;channel<6;++channel)for(unsigned lights=0;lights<256;++lights)for(unsigned diffuse=0;diffuse<3;++diffuse)for(unsigned attenuation=0;attenuation<3;++attenuation){
        GDSetCurrOffset(0);for(unsigned i=0;i<4;++i)GDWriteXFCmd(0x100e + i,0xa0000+i);state.consume(commands,GDGetCurrOffset());GDSetCurrOffset(0);
        const unsigned enabled=lights&1,ambient=(lights>>1)&1,material=(lights>>2)&1;
        GXSetChanCtrl(GXChannelID(channel),GXBool(enabled),GXColorSrc(ambient),GXColorSrc(material),lights|0x10000,GXDiffuseFn(diffuse),GXAttnFn(attenuation));
        assert(GDGetCurrOffset()==(channel>=4?18:9));state.consume(commands,GDGetCurrOffset());
        for(unsigned i=0;i<4;++i){auto value=state.xf(0x100e + i);if(i==(channel&3) || (channel>=4 && i==(channel&3)+2)){assert((value&1)==material && ((value>>1)&1)==enabled && ((value>>6)&1)==ambient);assert(((value>>2)&15)==(lights&15) && ((value>>11)&15)==(lights>>4));assert(((value>>7)&3)==(attenuation==0?0:diffuse));assert(((value>>9)&1)==(attenuation!=2) && ((value>>10)&1)==(attenuation!=0));}else assert(value==0xa0000+i);}
    }
    auto beforeChannel=GDGetCurrOffset();bool invalidChannel=false;try{GXSetChanCtrl(GXChannelID(6),1,GX_SRC_REG,GX_SRC_REG,0,GX_DF_NONE,GX_AF_NONE);}catch(const std::invalid_argument&){invalidChannel=true;}assert(invalidChannel && GDGetCurrOffset()==beforeChannel);
    std::puts("PASS channel-control routing, 13824 light/diffuse/attenuation configurations and untouched channels");
    const auto savedTextureCounts=state.bp(0)&~15u;
    for(unsigned count=0;count<=8;++count){GDSetCurrOffset(0);GXSetNumTexGens(u8(count));assert(GDGetCurrOffset()==19);state.consume(commands,GDGetCurrOffset());assert((state.bp(0)&15)==count && state.xf(0x103f)==count && (state.bp(0)&~15u)==savedTextureCounts);}
    const auto savedChannelCounts=state.bp(0)&~0x70u;
    for(unsigned count=0;count<=2;++count){GDSetCurrOffset(0);GXSetNumChans(u8(count));assert(GDGetCurrOffset()==19);state.consume(commands,GDGetCurrOffset());assert(((state.bp(0)>>4)&7)==count && state.xf(0x1009)==count && (state.bp(0)&~0x70u)==savedChannelCounts);}
    for(unsigned count:{9u,255u}){auto before=GDGetCurrOffset();bool invalid=false;try{GXSetNumTexGens(u8(count));}catch(const std::invalid_argument&){invalid=true;}assert(invalid && GDGetCurrOffset()==before);}
    for(unsigned count:{3u,255u}){auto before=GDGetCurrOffset();bool invalid=false;try{GXSetNumChans(u8(count));}catch(const std::invalid_argument&){invalid=true;}assert(invalid && GDGetCurrOffset()==before);}
    std::puts("PASS texture-generator/color-channel BP and XF count synchronization");
    for(unsigned mode:{0u,1u,0u}){GDSetCurrOffset(0);GXSetClipMode(GXClipMode(mode));assert(GDGetCurrOffset()==9);state.consume(commands,GDGetCurrOffset());assert(state.xf(0x1005)==mode);}
    auto clipPrior=GDGetCurrOffset();bool invalidClip=false;try{GXSetClipMode(GXClipMode(2));}catch(const std::invalid_argument&){invalidClip=true;}assert(invalidClip && GDGetCurrOffset()==clipPrior);
    std::puts("PASS native XF clip-mode enable/disable/reset and invalid enum rejection");
    GDSetCurrOffset(0);GXClearVtxDesc();state.consume(commands,GDGetCurrOffset());assert(state.cp(0x50)==512 && state.cp(0x60)==0 && state.xf(0x1008)==0);
    GDSetCurrOffset(0);GXSetVtxDesc(GX_VA_CLR1,GX_DIRECT);GXSetVtxDesc(GX_VA_TEX7,GX_INDEX16);GXSetVtxDesc(GX_VA_NBT,GX_INDEX8);state.consume(commands,GDGetCurrOffset());assert(state.cp(0x50)==(512u|(1u<<15)|(2u<<11)) && state.cp(0x60)==(3u<<14) && state.xf(0x1008)==(2u|8u|128u));
    GDSetCurrOffset(0);GXSetVtxDesc(GX_VA_NRM,GX_NONE);state.consume(commands,GDGetCurrOffset());assert(((state.xf(0x1008)>>2)&3)==2);GXSetVtxDesc(GX_VA_NBT,GX_NONE);
    auto descriptorPrior=GDGetCurrOffset();bool invalidDescriptor=false;try{GXSetVtxDesc(GX_VA_PNMTXIDX,GX_INDEX16);}catch(const std::invalid_argument&){invalidDescriptor=true;}assert(invalidDescriptor && GDGetCurrOffset()==descriptorPrior);
    std::puts("PASS retained GX vertex descriptor: sparse color/texture counts, NBT routing and invalid matrix mode");
    for(unsigned slot=0;slot<8;++slot){GDSetCurrOffset(0);GXSetVtxAttrFmt(GXVtxFmt(slot),GX_VA_POS,GX_POS_XYZ,GX_F32,u8(slot));GXSetVtxAttrFmt(GXVtxFmt(slot),GX_VA_TEX4,GX_TEX_ST,GX_S16,u8(31-slot));state.consume(commands,GDGetCurrOffset());assert((state.cp(0x70+slot)&511u)==(1u|(4u<<1)|(slot<<4)));assert(((state.cp(0x80+slot)>>27)&15u)==(1u|(3u<<1)));assert((state.cp(0x90+slot)&31u)==31-slot);}
    auto formatPrior=GDGetCurrOffset();bool invalidFormat=false;try{GXSetVtxAttrFmt(GX_VTXFMT0,GX_VA_POS,GX_POS_XYZ,GX_F32,32);}catch(const std::invalid_argument&){invalidFormat=true;}assert(invalidFormat && GDGetCurrOffset()==formatPrior);
    std::puts("PASS native VAT format banks: source-derived position fields, split TEX4 packing and invalid fraction");
    for(unsigned dst=0;dst<8;++dst){GDSetCurrOffset(0);GXSetTexCoordGen2(GXTexCoordID(dst),GX_TG_MTX2x4,GX_TG_TEX0,GX_IDENTITY,GX_FALSE,GX_PTIDENTITY);assert(GDGetCurrOffset()==33);state.consume(commands,GDGetCurrOffset());assert(state.xf(0x1040+dst)==(5u<<7));assert(state.xf(0x1050+dst)==61);unsigned bank=dst/4,shift=bank?(dst%4)*6:6+dst*6;assert(((state.cp(bank?0x40:0x30)>>shift)&63)==60);assert(state.cp(bank?0x40:0x30)==state.xf(0x1018+bank));}
    auto texgenPrior=GDGetCurrOffset();bool invalidTexgen=false;try{GXSetTexCoordGen2(GX_TEXCOORD0,GX_TG_BUMP0,GX_TG_POS,GX_IDENTITY,GX_FALSE,GX_PTIDENTITY);}catch(const std::invalid_argument&){invalidTexgen=true;}assert(invalidTexgen && GDGetCurrOffset()==texgenPrior);
    std::puts("PASS native texture generator: original font identity setup, eight destinations and CP/XF matrix indices");
    const unsigned expectedRows[21]={0,1,3,4,5,6,7,8,9,10,11,12,5,5,5,5,5,5,5,2,2};
    for(unsigned destination=0;destination<8;++destination)for(unsigned source=0;source<=20;++source)for(unsigned projection=0;projection<2;++projection){unsigned bank=destination/4,shift=bank?(destination%4)*6:6+destination*6;u32 priorIndices=state.cp(bank?0x40:0x30);u32 matrix=3*((source+destination)%21);GDSetCurrOffset(0);GXSetTexCoordGen2(GXTexCoordID(destination),GXTexGenType(projection),GXTexGenSrc(source),matrix,GX_TRUE,GX_PTTEXMTX0+3*(source%20));state.consume(commands,GDGetCurrOffset());assert(state.xf(0x1040+destination)==((projection==0?2u:0u)|(source<4?4u:0u)|(expectedRows[source]<<7)));assert(state.xf(0x1050+destination)==(3*(source%20)|256));assert((state.cp(bank?0x40:0x30)&~(63u<<shift))==(priorIndices&~(63u<<shift)));assert(((state.cp(bank?0x40:0x30)>>shift)&63)==matrix);}
    for(unsigned light=0;light<8;++light)for(unsigned source=0;source<7;++source){GDSetCurrOffset(0);GXSetTexCoordGen2(GX_TEXCOORD7,GXTexGenType(GX_TG_BUMP0+light),GXTexGenSrc(GX_TG_TEXCOORD0+source),GX_IDENTITY,GX_FALSE,GX_PTIDENTITY);state.consume(commands,GDGetCurrOffset());assert(state.xf(0x1047)==((5u<<7)|(1u<<4)|(source<<12)|(light<<15)));}
    for(unsigned color=0;color<2;++color){GDSetCurrOffset(0);GXSetTexCoordGen2(GX_TEXCOORD0,GX_TG_SRTG,GXTexGenSrc(GX_TG_COLOR0+color),GX_IDENTITY,GX_FALSE,GX_PTIDENTITY);state.consume(commands,GDGetCurrOffset());assert(state.xf(0x1040)==((2u<<7)|((2u+color)<<4)));}
    std::puts("PASS texgen matrix projection/source rows, bump light/source fields, color generation and retained matrix fields");
    auto textureIndices=state.cp(0x30)&~63u;for(unsigned id=0;id<=27;id+=3){GDSetCurrOffset(0);GXSetCurrentMtx(id);assert(GDGetCurrOffset()==15);state.consume(commands,GDGetCurrOffset());assert(state.cp(0x30)==(textureIndices|id) && state.xf(0x1018)==state.cp(0x30));GDSetCurrOffset(0);GXSetTexCoordGen2(GX_TEXCOORD0,GX_TG_MTX2x4,GX_TG_TEX0,GX_IDENTITY,GX_FALSE,GX_PTIDENTITY);state.consume(commands,GDGetCurrOffset());assert((state.cp(0x30)&63)==id);textureIndices=state.cp(0x30)&~63u;}
    for(unsigned invalid:{1u,30u,0xffffffffu}){auto prior=GDGetCurrOffset();bool rejected=false;try{GXSetCurrentMtx(invalid);}catch(const std::invalid_argument&){rejected=true;}assert(rejected && GDGetCurrOffset()==prior);}
    std::puts("PASS native current matrix: ten slots, CP/XF synchronization and texture-index preservation");
    for(unsigned slot=0;slot<10;++slot){float matrix[3][4]={{2,0,0,float(slot)},{0,3,0,-4},{0,0,5,6}};GDSetCurrOffset(0);GXLoadPosMtxImm(matrix,slot*3);assert(GDGetCurrOffset()==53);state.consume(commands,GDGetCurrOffset());auto transformed=state.transformPosition(slot*3,{1,2,3});assert(transformed[0]==2+slot && transformed[1]==2 && transformed[2]==21);}
    auto uploadPrior=GDGetCurrOffset();bool invalidUpload=false;try{GXLoadPosMtxImm(nullptr,0);}catch(const std::invalid_argument&){invalidUpload=true;}assert(invalidUpload && GDGetCurrOffset()==uploadPrior);
    std::puts("PASS original-layout position matrix uploads: ten slots and native affine transform evaluation");
    {Native::GxCommandState partial;bool unloaded=false;try{partial.transformPosition(0,{1,2,3});}catch(const std::runtime_error&){unloaded=true;}assert(unloaded);GDSetCurrOffset(0);GDWriteXFCmd(0,0);partial.consume(commands,GDGetCurrOffset());bool incomplete=false;try{partial.transformPosition(0,{1,2,3});}catch(const std::runtime_error&){incomplete=true;}assert(incomplete);float identity[3][4]={{1,0,0,0},{0,1,0,0},{0,0,1,0}};GDSetCurrOffset(0);GXLoadPosMtxImm(identity,0);bool truncated=false;try{partial.consume(commands,GDGetCurrOffset()-1);}catch(const std::exception&){truncated=true;}assert(truncated);incomplete=false;try{partial.transformPosition(0,{1,2,3});}catch(const std::runtime_error&){incomplete=true;}assert(incomplete);partial.consume(commands,GDGetCurrOffset());assert((partial.transformPosition(0,{1,2,3})==std::array<float,3>{1,2,3}));}
    std::puts("PASS XF matrix validity: unloaded/partial rejection and truncated-upload rollback");
    GDSetCurrOffset(0);GDWriteBPCmd(0xfeffffff);GDWriteBPCmd(0x40abcd00);GDWriteBPCmd(0x43123495);state.consume(commands,GDGetCurrOffset());
    for(unsigned enabled=0;enabled<2;++enabled)for(unsigned function=0;function<8;++function)for(unsigned update=0;update<2;++update){GDSetCurrOffset(0);GXSetZMode(GXBool(enabled),GXCompare(function),GXBool(update));assert(GDGetCurrOffset()==10);state.consume(commands,GDGetCurrOffset());assert(state.bp(0x40)==(0xabcd00u|enabled|(function<<1)|(update<<4)));}
    for(unsigned before:{0u,1u,0u}){GDSetCurrOffset(0);GXSetZCompLoc(GXBool(before));state.consume(commands,GDGetCurrOffset());assert(state.bp(0x43)==(0x123495u|(before<<6)));}
    for(unsigned bad:{2u,255u}){auto prior=GDGetCurrOffset();bool rejected=false;try{GXSetZCompLoc(GXBool(bad));}catch(const std::invalid_argument&){rejected=true;}assert(rejected && GDGetCurrOffset()==prior);rejected=false;try{GXSetZMode(GXBool(bad),GX_LEQUAL,GX_TRUE);}catch(const std::invalid_argument&){rejected=true;}assert(rejected && GDGetCurrOffset()==prior);}
    auto depthPrior=GDGetCurrOffset();bool invalidDepthFunction=false;try{GXSetZMode(GX_TRUE,GXCompare(8),GX_TRUE);}catch(const std::invalid_argument&){invalidDepthFunction=true;}assert(invalidDepthFunction && GDGetCurrOffset()==depthPrior);
    std::puts("PASS depth mode: all 32 configurations, comparison timing and unrelated BP preservation");
    GXFogAdjTable fogTable{};for(unsigned i=0;i<10;++i)fogTable.r[i]=u16(0x100+i*37);GDSetCurrOffset(0);GXSetFogRangeAdj(GX_TRUE,320,&fogTable);assert(GDGetCurrOffset()==30);state.consume(commands,GDGetCurrOffset());assert(state.bp(0xe8)==(662u|1024u));for(unsigned i=0;i<5;++i)assert(state.bp(0xe9+i)==(u32(fogTable.r[i*2])|(u32(fogTable.r[i*2+1])<<12)));
    GDSetCurrOffset(0);GXSetFogRangeAdj(GX_FALSE,0,nullptr);assert(GDGetCurrOffset()==5);state.consume(commands,GDGetCurrOffset());assert(state.bp(0xe8)==342);assert(state.bp(0xe9)==(u32(fogTable.r[0])|(u32(fogTable.r[1])<<12)));
    auto fogPrior=GDGetCurrOffset();bool missingFogTable=false;try{GXSetFogRangeAdj(GX_TRUE,0,nullptr);}catch(const std::invalid_argument&){missingFogTable=true;}assert(missingFogTable && GDGetCurrOffset()==fogPrior);fogTable.r[9]=4096;bool badFogField=false;try{GXSetFogRangeAdj(GX_TRUE,0,&fogTable);}catch(const std::invalid_argument&){badFogField=true;}assert(badFogField && GDGetCurrOffset()==fogPrior);
    std::puts("PASS fog range adjustment: paired table fields, center bias, disabled/null path and validation");
    GDSetCurrOffset(0);GXSetFog(GX_FOG_NONE,0,0,0,0,GXColor{0x12,0x34,0x56,0x78});assert(GDGetCurrOffset()==25);state.consume(commands,GDGetCurrOffset());assert(state.bp(0xee)==0 && state.bp(0xef)==4194319 && state.bp(0xf0)==1 && state.bp(0xf1)==0 && state.bp(0xf2)==0x123456);
    GDSetCurrOffset(0);GXSetFog(GX_FOG_ORTHO_LIN,2,6,0,8,GXColor{});state.consume(commands,GDGetCurrOffset());assert(state.bp(0xee)==(128u<<11));assert(state.bp(0xf1)==((126u<<11)|(1u<<20)|(2u<<21)));
    auto equationPrior=GDGetCurrOffset();bool nonfiniteFog=false;try{GXSetFog(GX_FOG_PERSP_LIN,std::numeric_limits<float>::infinity(),6,1,8,GXColor{});}catch(const std::invalid_argument&){nonfiniteFog=true;}assert(nonfiniteFog && GDGetCurrOffset()==equationPrior);
    std::puts("PASS native fog coefficients: disabled setup, orthographic float encoding, RGB byte order and finite validation");
    for(bool material:{false,true}){auto setColor=material?GXSetChanMatColor:GXSetChanAmbColor;u16 base=material?0x100c:0x100a;
    GDSetCurrOffset(0);setColor(GX_COLOR0A0,GXColor{0x12,0x34,0x56,0x78});setColor(GX_COLOR1A1,GXColor{0x9a,0xbc,0xde,0xf0});state.consume(commands,GDGetCurrOffset());assert(state.xf(base)==0x12345678 && state.xf(base+1)==0x9abcdef0);
    GDSetCurrOffset(0);setColor(GX_COLOR0,GXColor{1,2,3,4});setColor(GX_ALPHA1,GXColor{5,6,7,8});state.consume(commands,GDGetCurrOffset());assert(state.xf(base)==0x01020378 && state.xf(base+1)==0x9abcde08);
    GDSetCurrOffset(0);setColor(GX_ALPHA0,GXColor{0,0,0,0x22});setColor(GX_COLOR1,GXColor{0x44,0x55,0x66,0});state.consume(commands,GDGetCurrOffset());assert(state.xf(base)==0x01020322 && state.xf(base+1)==0x44556608);
    auto prior=GDGetCurrOffset();bool rejected=false;try{setColor(GXChannelID(6),GXColor{});}catch(const std::invalid_argument&){rejected=true;}assert(rejected && GDGetCurrOffset()==prior);}
    std::puts("PASS native ambient/material RGBA packing and independent RGB/alpha channel updates");
    const auto preservedIndirect=state.bp(0)&~0x70000u;
    for(unsigned count=0;count<=4;++count){GDSetCurrOffset(0);GXSetNumIndStages(u8(count));assert(GDGetCurrOffset()==10);assert(word(commands)==0xfe070000u && word(commands+5)==(count<<16));state.consume(commands,GDGetCurrOffset());assert(((state.bp(0)>>16)&7)==count && (state.bp(0)&~0x70000u)==preservedIndirect);}
    for(unsigned count:{5u,7u,255u}){auto before=GDGetCurrOffset();bool invalid=false;try{GXSetNumIndStages(u8(count));}catch(const std::invalid_argument&){invalid=true;}assert(invalid && GDGetCurrOffset()==before);}
    std::puts("PASS indirect-stage count field, unrelated-state preservation and invalid-count rejection");
    for(unsigned stage : {0u,1u,15u}){
        const unsigned sourceColor=stage==0 ? GX_CC_RASC : GX_CC_CPREV;
        const unsigned sourceAlpha=stage==0 ? GX_CA_RASA : GX_CA_APREV;
        const std::array<unsigned,4> colorModes[]={
            {GX_CC_ZERO,GX_CC_TEXC,sourceColor,GX_CC_ZERO},
            {sourceColor,GX_CC_TEXC,GX_CC_TEXA,GX_CC_ZERO},
            {sourceColor,GX_CC_ONE,GX_CC_TEXC,GX_CC_ZERO},
            {GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,GX_CC_TEXC},
            {GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,sourceColor}};
        const std::array<unsigned,4> alphaModes[]={
            {GX_CA_ZERO,GX_CA_TEXA,sourceAlpha,GX_CA_ZERO},
            {GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO,sourceAlpha},
            {GX_CA_ZERO,GX_CA_TEXA,sourceAlpha,GX_CA_ZERO},
            {GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO,GX_CA_TEXA},
            {GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO,sourceAlpha}};
        for(unsigned mode=0;mode<5;++mode){
            GDSetCurrOffset(0);
            GXSetTevSwapMode(GXTevStageID(stage),GX_TEV_SWAP3,GX_TEV_SWAP2);
            GXSetTevOp(GXTevStageID(stage),GXTevMode(mode));
            state.consume(commands,GDGetCurrOffset());
            const auto preset=state.tevStage(stage);
            assert(preset.color.inputs==colorModes[mode] && preset.alpha.inputs==alphaModes[mode]);
            assert(preset.rasterSwap==3 && preset.textureSwap==2);
            assert(preset.color.bias==0 && preset.alpha.bias==0 && preset.color.scale==0 && preset.alpha.scale==0);
            assert(preset.color.destination==0 && preset.alpha.destination==0);
            assert(preset.color.clamp && preset.alpha.clamp && !preset.color.subtract && !preset.alpha.subtract);
        }
    }
    const GXTexFmt depthFormats[]={GX_TF_Z8,GX_TF_Z16,GX_TF_Z24X8,GX_TF_RGBA8};
    for(unsigned format=0;format<4;++format){
        for(unsigned operation=0;operation<3;++operation){
            GDSetCurrOffset(0);
            GXSetZTexture(GXZTexOp(operation),depthFormats[format],0xab123456);
            assert(word(commands)==0xf4123456);
            assert(word(commands+5)==(0xf5000000u|(format<2 ? format : 2)|(operation<<2)));
            state.consume(commands,GDGetCurrOffset());
            const auto depth=state.depthTexture();
            assert(depth.bias==0x123456 && depth.type==(format<2 ? format : 2) && depth.operation==operation);
        }
    }
    const auto depthOffset=GDGetCurrOffset();
    bool badDepth=false;
    try{GXSetZTexture(GXZTexOp(3),GX_TF_Z8,0);}catch(const std::invalid_argument&){badDepth=true;}
    assert(badDepth && GDGetCurrOffset()==depthOffset);
    // Restore the texture command fixture overwritten by the stage checks.
    GDSetCurrOffset(0);
    for(unsigned i=0;i<8;++i){
        J3DGDSetTexImgAttr(GXTexMapID(i),64,32,GX_TF_RGBA8);
        J3DGDSetTexImgPtr(GXTexMapID(i),texture+32);
    }
    state.consume(commands, GDGetCurrOffset());
    for (unsigned i=0;i<8;++i) {
        const auto tex = state.texture(i);
        assert(tex.width == 64 && tex.height == 32 && tex.format == 6);
        assert(tex.address == address+32);
        assert(*resources.resolve(tex.address, 1) == 0x7b);
    }
    GDSetCurrOffset(0);
    GDWriteBPCmd(0xfe00000f);
    GDWriteBPCmd(0x88000002);
    GDWriteBPCmd(0x89000005);
    GDWriteCPCmd(0xa0,0x12345678);
    GDWriteXFCmd(0x1000,0x3f800000);
    state.consume(commands, GDGetCurrOffset());
    assert(state.texture(0).width == 51 && state.texture(0).height == 32);
    assert(state.texture(1).width == 6 && state.texture(1).height == 1);
    assert(state.cp(0xa0) == 0x12345678 && state.xf(0x1000) == 0x3f800000);
    const u8 invalid[] = {0x61,0x88,0,0,0,0x80};
    bool malformed = false;
    try { state.consume(invalid,sizeof invalid); } catch (const std::length_error&) { malformed = true; }
    assert(malformed && state.texture(0).width == 51);
    const u8 truncated[] = {0x61,0x88,0};
    malformed = false;
    try { state.consume(truncated,sizeof truncated); } catch (const std::length_error&) { malformed = true; }
    assert(malformed && state.texture(0).width == 51);
    const u8 badXf[] = {0x10,0,1,0x10,0xff,0,0,0,0,0,0,0,0};
    malformed = false;
    try { state.consume(badXf,sizeof badXf); } catch (const std::out_of_range&) { malformed = true; }
    assert(malformed && state.xf(0x1000) == 0x3f800000);
    const u8 maskOnly[] = {0x61,0xfe,0,0,0x0f};
    const u8 maskedAttr[] = {0x61,0x88,0,0,3};
    state.consume(maskOnly,sizeof maskOnly);
    state.consume(maskedAttr,sizeof maskedAttr);
    assert(state.texture(0).width == 52 && state.texture(0).height == 32);
    GDSetCurrOffset(80);
    GDSetCurrOffset(0);
    for (unsigned i=0;i<16;++i) {
        texture[32+i*2] = 255;
        texture[33+i*2] = 10+i;
        // The RGBA8 tile's GB plane lies outside this fixture's first plane.
    }
    // Refresh the snapshot before generating a new command address.
    resources.remove(texture);
    resources.add(texture,sizeof texture);
    J3DGDSetTexImgAttr(GX_TEXMAP0,4,4,GX_TF_RGBA8);
    J3DGDSetTexImgPtr(GX_TEXMAP0,texture+32);
    state.consume(commands,GDGetCurrOffset());
    auto image = decodeCommandTexture(state,0,resources);
    assert(image.width == 4 && image.height == 4 && image.pixels.size() == 16);
    for (unsigned i=0;i<16;++i) {
        assert(image.pixels[i].a == 255 && image.pixels[i].r == 10+i);
        assert(image.pixels[i].g == 0 && image.pixels[i].b == 0);
    }
    verifyRendering(image,true);
    verifyRendering(image,false);
    bool shortImage = false;
    GDSetCurrOffset(0);
    J3DGDSetTexImgAttr(GX_TEXMAP0,8,8,GX_TF_RGBA8);
    state.consume(commands,GDGetCurrOffset());
    try { decodeCommandTexture(state,0,resources); }
    catch (const std::out_of_range&) { shortImage = true; }
    assert(shortImage);
    GDSetCurrOffset(0);
    Mtx matrix = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
    J3DFifoLoadPosMtxImm(matrix,3);
    J3DFifoLoadNrmMtxImm(matrix,3);
    state.consume(commands,GDGetCurrOffset());
    for (unsigned r=0;r<3;++r) for (unsigned c=0;c<4;++c) {
        float decoded;
        const auto bits=state.xf(12+r*4+c);
        std::memcpy(&decoded,&bits,4);
        assert(decoded==matrix[r][c]);
    }
    for (unsigned r=0;r<3;++r) for (unsigned c=0;c<3;++c) {
        float decoded;
        const auto bits=state.xf(0x409+r*3+c);
        std::memcpy(&decoded,&bits,4);
        assert(decoded==matrix[r][c]);
    }
    alignas(32) u8 indices[32] = {};
    alignas(32) u8 matrixArray[96] = {};
    for (unsigned i=0;i<12;++i) {
        u32 bits;
        const float value = 100.0f+i;
        std::memcpy(&bits,&value,4);
        for (unsigned j=0;j<4;++j) matrixArray[48+i*4+j] = bits >> (24-j*8);
    }
    const auto matrixAddress = resources.add(matrixArray,sizeof matrixArray);
    matrixArray[51] ^= 1;
    assert(resources.resolve(matrixAddress+51,1)[0] != matrixArray[51]);
    DCStoreRangeNoSync(matrixArray+51,1);
    assert(resources.resolve(matrixAddress+51,1)[0] == matrixArray[51]);
    assert(resources.address(matrixArray) == matrixAddress);
    matrixArray[51] ^= 1;
    DCFlushRange(matrixArray+48,4);
    GDSetCurrOffset(0);
    GXSetArray(GX_POS_MTX_ARRAY,matrixArray,48);
    J3DFifoLoadIndx(0x20,1,(11<<12)|24);
    GXSetArray(GX_NRM_MTX_ARRAY,matrixArray,48);
    J3DFifoLoadNrmMtxIndx3x3(1,3);
    state.consume(commands,GDGetCurrOffset(),&resources);
    for (unsigned i=0;i<12;++i) {
        const auto bits = state.xf(24+i);
        float value;
        std::memcpy(&value,&bits,4);
        assert(value == 100.0f+i);
    }
    for (unsigned i=0;i<9;++i) assert(state.xf(0x409+i)==state.xf(24+i));
    GDSetCurrOffset(0);
    J3DFifoLoadIndx(0x20,2,(11<<12)|24);
    bool invalidIndex = false;
    try { state.consume(commands,GDGetCurrOffset(),&resources); }
    catch (const std::out_of_range&) { invalidIndex = true; }
    assert(invalidIndex && state.xf(24)==state.xf(0x409));
    GDSetCurrOffset(0);
    GXSetArray(GX_VA_NBT,matrixArray+4,12);
    state.consume(commands,GDGetCurrOffset());
    assert(state.cp(0xa1)==matrixAddress+4 && state.cp(0xb1)==12);
    const auto previousOffset=GDGetCurrOffset();
    bool invalidAttr=false;
    try { GXSetArray(GX_VA_PNMTXIDX,matrixArray,12); }
    catch(const std::invalid_argument&) { invalidAttr=true; }
    assert(invalidAttr && GDGetCurrOffset()==previousOffset);
    resources.remove(matrixArray);
    GDSetCurrOffset(0);
    const GXVtxDescList descriptor[] = {{GX_VA_PNMTXIDX,GX_DIRECT},{GX_VA_POS,GX_INDEX16},
        {GX_VA_NRM,GX_INDEX8},{GX_VA_CLR0,GX_DIRECT},{GX_VA_TEX0,GX_INDEX16},{GX_VA_NULL,GX_NONE}};
    GDSetVtxDescv(descriptor);
    state.consume(commands,GDGetCurrOffset());
    assert((state.cp(0x50)&1)==1);
    assert(((state.cp(0x50)>>9)&3)==GX_INDEX16);
    assert(((state.cp(0x50)>>11)&3)==GX_INDEX8);
    assert(((state.cp(0x50)>>13)&3)==GX_DIRECT);
    assert((state.cp(0x60)&3)==GX_INDEX16);
    assert(state.xf(0x1008)==(1u | 1u<<2 | 1u<<4));
    GDSetCurrOffset(0);
    const GXVtxAttrFmtList formats[]={{GX_VA_POS,GX_POS_XYZ,GX_S16,7},
        {GX_VA_NRM,GX_NRM_XYZ,GX_F32,0},{GX_VA_NULL,GX_POS_XY,GX_U8,0}};
    J3DGDSetVtxAttrFmtv(GX_VTXFMT3,formats,false);
    state.consume(commands,GDGetCurrOffset());
    const auto positionFormat=state.positionFormat(3);
    assert(positionFormat.components==3&&positionFormat.type==GX_S16&&positionFormat.fraction==7);
    assert(positionFormat.descriptor==GX_INDEX16);
    const u8 positionBytes[]={0,0,0,0,0,0,0xff,0x80,0x01,0x00,0xff,0xc0};
    resources.add(positionBytes,sizeof positionBytes);
    GDSetCurrOffset(0);GXSetArray(GX_VA_POS,positionBytes,6);
    state.consume(commands,GDGetCurrOffset());
    const auto point=state.indexedPosition(3,1,resources);
    assert(point[0]==-1 && point[1]==2 && point[2]==-.5f);
    bool badPosition=false;
    try{state.indexedPosition(3,2,resources);}catch(const std::out_of_range&){badPosition=true;}
    assert(badPosition);
    resources.remove(positionBytes);
    const u8 bytePositions[]={128,64,32};resources.add(bytePositions,sizeof bytePositions);
    GDSetCurrOffset(0);GXSetArray(GX_VA_POS,bytePositions,3);
    const GXVtxAttrFmtList byteFormats[]={{GX_VA_POS,GX_POS_XYZ,GX_S8,7},{GX_VA_NULL,GX_POS_XY,GX_U8,0}};
    J3DGDSetVtxAttrFmtv(GX_VTXFMT0,byteFormats,false);
    state.consume(commands,GDGetCurrOffset());
    assert((state.indexedPosition(0,0,resources)==std::array<float,3>{-1,.5f,.25f}));
    GDSetCurrOffset(0);GDWriteCPCmd(0x70,state.cp(0x70)&~(1u<<30));
    state.consume(commands,GDGetCurrOffset());
    assert((state.indexedPosition(0,0,resources)==std::array<float,3>{-128,64,32}));
    resources.remove(bytePositions);
    GDSetCurrOffset(0);
    GXSetIndTexOrder(GX_INDTEXSTAGE0,GX_TEXCOORD3,GX_TEXMAP2);
    GXSetIndTexOrder(GX_INDTEXSTAGE3,GX_TEXCOORD7,GX_TEXMAP6);
    GXSetIndTexOrder(GX_INDTEXSTAGE1,GX_TEXCOORD_NULL,GX_TEXMAP_NULL);
    state.consume(commands,GDGetCurrOffset());
    assert((state.bp(0x27)&63)==(2|3<<3));
    assert(((state.bp(0x27)>>18)&63)==(6|7<<3));
    assert(((state.bp(0x27)>>6)&63)==0);
    GDSetCurrOffset(0);
    GXSetTevIndirect(GX_TEVSTAGE3,GX_INDTEXSTAGE2,GX_ITF_5,GX_ITB_STU,GX_ITM_T1,
        GX_ITW_64,GX_ITW_16,GX_TRUE,GX_TRUE,GX_ITBA_U);
    assert(word(commands)==IND_CMD(2,GX_ITF_5,GX_ITB_STU,GX_ITBA_U,GX_ITM_T1,GX_ITW_64,GX_ITW_16,1,1,0x13));
    state.consume(commands,GDGetCurrOffset());
    assert(state.bp(0x13)==(word(commands)&0xffffff));
    auto indirectState=state.tevIndirect(3);
    assert(indirectState.stage==2 && indirectState.format==GX_ITF_5 && indirectState.bias==GX_ITB_STU);
    assert(indirectState.alpha==GX_ITBA_U && indirectState.matrix==GX_ITM_T1);
    assert(indirectState.wrapS==GX_ITW_64 && indirectState.wrapT==GX_ITW_16);
    assert(indirectState.unmodifiedLod && indirectState.addPrevious);
    const auto indirectOffset=GDGetCurrOffset();
    bool badIndirect=false;
    try{GXSetTevIndirect(GX_TEVSTAGE0,GX_INDTEXSTAGE0,GX_ITF_8,GX_ITB_NONE,GXIndTexMtxID(4),
        GX_ITW_OFF,GX_ITW_OFF,GX_FALSE,GX_FALSE,GX_ITBA_OFF);}
    catch(const std::invalid_argument&){badIndirect=true;}
    assert(badIndirect && GDGetCurrOffset()==indirectOffset);
    GDSetCurrOffset(0);GDWriteBPCmd(0x14054321);
    GXSetTevDirect(GX_TEVSTAGE3);
    state.consume(commands,GDGetCurrOffset());
    assert(state.bp(0x13)==0 && state.bp(0x14)==0x054321);
    indirectState=state.tevIndirect(3);
    assert(indirectState.matrix==0 && indirectState.bias==0 && !indirectState.addPrevious && !indirectState.unmodifiedLod);
    GDSetCurrOffset(0);
    GXSetTevColor(GX_TEVREG1,GXColor{17,33,65,129});
    GXSetTevKColorSel(GX_TEVSTAGE2,GXTevKColorSel(12));
    GXSetTevKAlphaSel(GX_TEVSTAGE3,GXTevKAlphaSel(19));
    state.consume(commands,GDGetCurrOffset());
    assert((state.bp(0xe4)&2047)==17&&((state.bp(0xe4)>>12)&2047)==129);
    assert((state.bp(0xe5)&2047)==65&&((state.bp(0xe5)>>12)&2047)==33);
    assert(((state.bp(0xf7)>>4)&31)==12&&((state.bp(0xf7)>>19)&31)==19);
    GDSetCurrOffset(0);
    GXSetTevKColor(GX_KCOLOR2,GXColor{3,5,7,11});
    GXSetTevColorS10(GX_TEVREG0,GXColorS10{-5,511,-512,255});
    state.consume(commands,GDGetCurrOffset());
    assert((state.tevColor(2)==std::array<int,4>{17,33,65,129}));
    assert((state.tevColor(2,true)==std::array<int,4>{3,5,7,11}));
    assert((state.tevColor(1)==std::array<int,4>{-5,511,-512,255}));
    for(unsigned id=0;id<4;++id){
        const auto regular=state.tevColor(id);
        GDSetCurrOffset(0);
        GXSetTevKColor(GXTevKColorID(id),GXColor{u8(13+id),u8(37+id),u8(91+id),u8(255-id)});
        assert(word(commands)==TEV_REGISTERL(13+id,255-id,1,0xe0+id*2));
        assert(word(commands+5)==TEV_REGISTERH(91+id,37+id,1,0xe1+id*2));
        state.consume(commands,GDGetCurrOffset());
        assert(state.tevColor(id)==regular);
        assert((state.tevColor(id,true)==std::array<int,4>{int(13+id),int(37+id),int(91+id),int(255-id)}));
    }
    const auto constantOffset=GDGetCurrOffset();
    bool invalidConstant=false;
    try{GXSetTevKColor(GXTevKColorID(-1),GXColor{});}
    catch(const std::invalid_argument&){invalidConstant=true;}
    assert(invalidConstant && GDGetCurrOffset()==constantOffset);
    const u8 childList[]={8,0xa0,0x12,0x34,0x56,0x78};
    resources.add(childList,sizeof childList);
    GDSetCurrOffset(0);
    GXCallDisplayList(childList,sizeof childList);
    state.consume(commands,GDGetCurrOffset(),&resources);
    assert(state.cp(0xa0)==0x12345678);
    resources.remove(childList);
    bool staleList=false;
    try{state.consume(commands,GDGetCurrOffset(),&resources);}catch(const std::out_of_range&){staleList=true;}
    assert(staleList && state.cp(0xa0)==0x12345678);
    u8 loopList[9]={};resources.add(loopList,sizeof loopList);
    GDSetCurrOffset(0);GXCallDisplayList(loopList,sizeof loopList);
    std::memcpy(loopList,commands,sizeof loopList);
    resources.refresh(loopList,sizeof loopList);
    bool loopRejected=false;
    try{state.consume(commands,GDGetCurrOffset(),&resources);}catch(const std::length_error&){loopRejected=true;}
    assert(loopRejected && state.cp(0xa0)==0x12345678);
    resources.remove(loopList);
    alignas(32) u8 palette[32] = {};
    for (unsigned i=0;i<32;++i) indices[i] = 0x12;
    palette[2] = 0xf8; palette[3] = 0; // RGB565 red
    palette[4] = 0x07; palette[5] = 0xe0; // RGB565 green
    resources.add(indices,sizeof indices);
    resources.add(palette,sizeof palette);
    GDSetCurrOffset(0);
    J3DGDLoadTlut(palette,0x80200,GX_TLUT_16);
    J3DGDSetTexTlut(GX_TEXMAP7,0x80200,GX_TL_RGB565);
    J3DGDSetTexImgAttr(GX_TEXMAP7,8,8,GXTexFmt(GX_TF_C4));
    J3DGDSetTexImgPtr(GX_TEXMAP7,indices);
    state.consume(commands,GDGetCurrOffset(),&resources);
    resources.remove(palette); // The transferred palette must own its bytes.
    auto indexed = decodeCommandTexture(state,7,resources);
    for (unsigned i=0;i<64;++i) {
        assert(indexed.pixels[i].r == (i%2 ? 0 : 255));
        assert(indexed.pixels[i].g == (i%2 ? 255 : 0));
    }
    verifyRendering(indexed,true);
    verifyRendering(indexed,false);
    const u8 invalidTransfer[] = {0x61,0x65,0x1f,0xff,0xff};
    bool invalidPalette = false;
    try { state.consume(invalidTransfer,sizeof invalidTransfer,&resources); }
    catch (const std::out_of_range&) { invalidPalette = true; }
    assert(invalidPalette && state.palette(7,16)[2] == 0xf8);
    resources.remove(indices);
    resources.remove(texture);
    GDSetCurrOffset(80);
    bool rejected = false;
    try { J3DGDSetTexImgPtr(GX_TEXMAP0, texture); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected && GDGetCurrOffset() == 80);
    GDSetCurrent(nullptr);
    GDInitGDLObj(&dl,commands,sizeof commands);GDSetCurrent(&dl);
    GXPosition3s16(-1,256,-32768);
    assert(commands[0]==255&&commands[1]==255&&commands[2]==1&&commands[3]==0&&commands[4]==128&&commands[5]==0);
    GXCmd1f32(1.0f);
    assert(commands[6]==0x3f&&commands[7]==0x80&&commands[8]==0&&commands[9]==0);
    GDSetCurrent(nullptr);
    std::puts("Original J3D texture commands: eight slots, state decoding, bounded RGBA8 pixels and D3D hardware/WARP readback passed");
    if (argc == 2) {
        WiiAssets assets(std::filesystem::u8path(argv[1]));
        Rarc archive(assets.readFile("/ObjectData/Mario.arc"));
        const auto modelBytes = archive.readFile("/mario.bdl");
        BinaryView model(modelBytes);
        const auto reference = decodeModel(model);
        size_t vertexSectionOffset=32;
        for(unsigned sectionNo=0;sectionNo<model.u32(12);++sectionNo){
            const auto vertexSection=model.sub(vertexSectionOffset,model.u32(vertexSectionOffset+4));
            if(vertexSection.tag(0)=="VTX1"){
                const auto formatOffset=vertexSection.u32(8);
                for(unsigned f=0;f<32;++f){
                    const auto entry=formatOffset+f*16;
                    const auto attr=vertexSection.u32(entry);
                    if(attr==255)break;
                    if(attr!=9)continue;
                    const unsigned components=vertexSection.u32(entry+4),type=vertexSection.u32(entry+8),fraction=vertexSection.u8(entry+12);
                    const unsigned width=type<2?1:type<4?2:4,stride=(components+2)*width;
                    const size_t offset=vertexSection.u32(12),size=reference.positions.size()*stride;
                    vertexSection.sub(offset,size);
                    std::vector<u8> positions(size);
                    for(size_t k=0;k<size;++k)positions[k]=vertexSection.u8(offset+k);
                    resources.add(positions.data(),positions.size());
                    GDInitGDLObj(&dl,commands,sizeof commands);GDSetCurrent(&dl);
                    const GXVtxDescList desc[]={{GX_VA_POS,GX_INDEX16},{GX_VA_NULL,GX_NONE}};
                    const GXVtxAttrFmtList fmt[]={{GX_VA_POS,GXCompCnt(components),GXCompType(type),u8(fraction)},
                        {GX_VA_NULL,GX_POS_XY,GX_U8,0}};
                    GDSetVtxDescv(desc);J3DGDSetVtxAttrFmtv(GX_VTXFMT0,fmt,false);
                    GXSetArray(GX_VA_POS,positions.data(),stride);
                    Native::GxCommandState vertexState;
                    vertexState.consume(commands,GDGetCurrOffset());
                    assert(reference.positions.size()<=65536);
                    for(unsigned k=0;k<reference.positions.size();++k)
                        assert(vertexState.indexedPosition(0,k,resources)==reference.positions[k]);
                    std::printf("PASS: %zu original Mario vertex positions via VCD/VAT/array commands\n",reference.positions.size());
                    resources.remove(positions.data());GDSetCurrent(nullptr);
                }
                break;
            }
            vertexSectionOffset+=vertexSection.size();
        }
        size_t sectionOffset = 32;
        for (unsigned i=0;i<model.u32(12);++i) {
            const auto section = model.sub(sectionOffset,model.u32(sectionOffset+4));
            if (section.tag(0) == "TEX1") {
                const unsigned count = section.u16(8);
                assert(count == reference.textures.size() && count > 0);
                for (unsigned i=0;i<count;++i) {
                    const size_t header = section.u32(12)+i*32;
                    const unsigned width = section.u16(header+2),height = section.u16(header+4),format = section.u8(header);
                    const auto encodedSize = gxEncodedSize(width,height,format);
                    const auto offset = header+size_t(section.u32(header+28));
                    section.sub(offset,encodedSize);
                    std::vector<u8> encoded(encodedSize);
                    for (size_t k=0;k<encodedSize;++k) encoded[k] = section.u8(offset+k);
                    resources.add(encoded.data(),encoded.size());
                    std::vector<u8> paletteBytes;
                    GDInitGDLObj(&dl,commands,sizeof commands);
                    GDSetCurrent(&dl);
                    if (format==8 || format==9 || format==10) {
                        const unsigned entries = section.u16(header+10);
                        assert(entries > 0 && entries <= 16384);
                        paletteBytes.resize(((entries+15)/16)*32);
                        const auto paletteOffset = header+size_t(section.u32(header+12));
                        section.sub(paletteOffset,entries*2);
                        for (unsigned k=0;k<entries*2;++k) paletteBytes[k] = section.u8(paletteOffset+k);
                        resources.add(paletteBytes.data(),paletteBytes.size());
                        J3DGDLoadTlut(paletteBytes.data(),0x80000,GXTlutSize((entries+15)/16));
                        J3DGDSetTexTlut(GX_TEXMAP0,0x80000,GXTlutFmt(section.u8(header+9)));
                    }
                    J3DGDSetTexImgAttr(GX_TEXMAP0,width,height,GXTexFmt(format));
                    J3DGDSetTexImgPtr(GX_TEXMAP0,encoded.data());
                    Native::GxCommandState assetState;
                    assetState.consume(commands,GDGetCurrOffset(),&resources);
                    const auto decoded = decodeCommandTexture(assetState,0,resources);
                    const auto& expected = reference.textures[i];
                    assert(decoded.pixels.size() == expected.pixels.size());
                    for (size_t k=0;k<decoded.pixels.size();++k) {
                        const auto a=decoded.pixels[k],b=expected.pixels[k];
                        assert(a.r==b.r && a.g==b.g && a.b==b.b && a.a==b.a);
                    }
                    verifyRendering(decoded,true);
                    verifyRendering(decoded,false);
                    resources.remove(encoded.data());
                    if (!paletteBytes.empty()) resources.remove(paletteBytes.data());
                    GDSetCurrent(nullptr);
                }
                std::printf("PASS: %u original Mario textures through J3D commands, native resources, pixel decode and hardware/WARP readback\n",count);
                break;
            }
            sectionOffset += section.size();
        }
    }
}


































