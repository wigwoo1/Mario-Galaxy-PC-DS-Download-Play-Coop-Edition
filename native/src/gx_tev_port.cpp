#include <JSystem/J3DGraphBase/J3DGD.hpp>
#include <revolution/gx/GXTev.h>
#include <stdexcept>

extern "C" void GXSetZTexture(GXZTexOp operation,GXTexFmt format,u32 bias) {
    if(unsigned(operation)>2)throw std::invalid_argument("GX depth texture operation");
    // Preserve the original SDK default format mapping and 24-bit bias field.
    const unsigned type=format==GX_TF_Z8 ? 0 : format==GX_TF_Z16 ? 1 : 2;
    GDOverflowCheck(10);
    GDWriteBPCmd(0xf4000000u|(bias&0xffffff));
    GDWriteBPCmd(0xf5000000u|type|(unsigned(operation)<<2));
}

extern "C" void GXSetTevOp(GXTevStageID stage,GXTevMode mode) {
    if(unsigned(stage)>15 || unsigned(mode)>4)throw std::invalid_argument("GX TEV preset mode");
    const auto color=stage==GX_TEVSTAGE0 ? GX_CC_RASC : GX_CC_CPREV;
    const auto alpha=stage==GX_TEVSTAGE0 ? GX_CA_RASA : GX_CA_APREV;
    // Preflight all four masked commands before emitting any part of the preset.
    GDOverflowCheck(40);
    switch(mode){
    case GX_MODULATE:
        GXSetTevColorIn(stage,GX_CC_ZERO,GX_CC_TEXC,color,GX_CC_ZERO);
        GXSetTevAlphaIn(stage,GX_CA_ZERO,GX_CA_TEXA,alpha,GX_CA_ZERO);break;
    case GX_DECAL:
        GXSetTevColorIn(stage,color,GX_CC_TEXC,GX_CC_TEXA,GX_CC_ZERO);
        GXSetTevAlphaIn(stage,GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO,alpha);break;
    case GX_BLEND:
        GXSetTevColorIn(stage,color,GX_CC_ONE,GX_CC_TEXC,GX_CC_ZERO);
        GXSetTevAlphaIn(stage,GX_CA_ZERO,GX_CA_TEXA,alpha,GX_CA_ZERO);break;
    case GX_REPLACE:
        GXSetTevColorIn(stage,GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,GX_CC_TEXC);
        GXSetTevAlphaIn(stage,GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO,GX_CA_TEXA);break;
    case GX_PASSCLR:
        GXSetTevColorIn(stage,GX_CC_ZERO,GX_CC_ZERO,GX_CC_ZERO,color);
        GXSetTevAlphaIn(stage,GX_CA_ZERO,GX_CA_ZERO,GX_CA_ZERO,alpha);break;
    }
    GXSetTevColorOp(stage,GX_TEV_ADD,GX_TB_ZERO,GX_CS_SCALE_1,GX_TRUE,GX_TEVPREV);
    GXSetTevAlphaOp(stage,GX_TEV_ADD,GX_TB_ZERO,GX_CS_SCALE_1,GX_TRUE,GX_TEVPREV);
}

extern "C" void GXSetNumTevStages(u8 stages) {
    if(stages<1 || stages>16)throw std::invalid_argument("GX TEV stage count");
    // Native command state commits immediately instead of using SDK dirtyState.
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe003c00);
    GDWriteBPCmd(unsigned(stages-1)<<10);
}

extern "C" void GXSetAlphaCompare(GXCompare first,u8 firstReference,GXAlphaOp operation,GXCompare second,u8 secondReference) {
    if(unsigned(first)>7 || unsigned(second)>7 || unsigned(operation)>3)
        throw std::invalid_argument("GX alpha comparison");
    GDOverflowCheck(5);
    GDWriteBPCmd(0xf3000000u|unsigned(firstReference)|(unsigned(secondReference)<<8)|
        (unsigned(first)<<16)|(unsigned(second)<<19)|(unsigned(operation)<<22));
}

extern "C" void GXSetTevSwapMode(GXTevStageID stage,GXTevSwapSel raster,GXTevSwapSel texture) {
    if(unsigned(stage)>15 || unsigned(raster)>3 || unsigned(texture)>3)
        throw std::invalid_argument("GX TEV swap selection");
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe00000f);
    GDWriteBPCmd(((0xc1u+unsigned(stage)*2)<<24)|unsigned(raster)|(unsigned(texture)<<2));
}
extern "C" void GXSetTevSwapModeTable(GXTevSwapSel table,GXTevColorChan red,GXTevColorChan green,GXTevColorChan blue,GXTevColorChan alpha) {
    if(unsigned(table)>3 || unsigned(red)>3 || unsigned(green)>3 || unsigned(blue)>3 || unsigned(alpha)>3)
        throw std::invalid_argument("GX TEV swap table");
    GDOverflowCheck(20);
    const unsigned reg=0xf6+unsigned(table)*2;
    GDWriteBPCmd(0xfe00000f);
    GDWriteBPCmd((reg<<24)|unsigned(red)|(unsigned(green)<<2));
    GDWriteBPCmd(0xfe00000f);
    GDWriteBPCmd(((reg+1)<<24)|unsigned(blue)|(unsigned(alpha)<<2));
}

extern "C" void GXSetTevOrder(GXTevStageID stage,GXTexCoordID coord,GXTexMapID map,GXChannelID color) {
    static constexpr unsigned channels[]={0,1,0,1,0,1,7,5,6};
    if(unsigned(stage)>15 || (color!=GX_COLOR_NULL && unsigned(color)>=9))
        throw std::invalid_argument("GX TEV order");
    const unsigned rawMap=unsigned(map);
    const unsigned texture=(rawMap & ~unsigned(GX_TEX_DISABLE))<8 ? rawMap&7 : 0;
    const unsigned coordinate=unsigned(coord)<8 ? unsigned(coord) : 0;
    const unsigned channel=color==GX_COLOR_NULL ? 7 : channels[unsigned(color)];
    const bool enabled=map!=GX_TEXMAP_NULL && !(rawMap & GX_TEX_DISABLE);
    const unsigned shift=(unsigned(stage)&1)*12;
    const unsigned fields=texture|(coordinate<<3)|(unsigned(enabled)<<6)|(channel<<7);
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe000000u|(0x3ffu<<shift));
    GDWriteBPCmd(((0x28u+unsigned(stage)/2)<<24)|(fields<<shift));
}

static void selectInputs(GXTevStageID stage,unsigned a,unsigned b,unsigned c,unsigned d,bool alpha) {
    const unsigned limit=alpha ? 7 : 15;
    if(unsigned(stage)>unsigned(GX_TEVSTAGE15) || a>limit || b>limit || c>limit || d>limit)
        throw std::invalid_argument("GX TEV input selection");
    const unsigned inputs=alpha ? (a<<13)|(b<<10)|(c<<7)|(d<<4) : (a<<12)|(b<<8)|(c<<4)|d;
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe000000u | (alpha ? 0xfff0u : 0xffffu));
    GDWriteBPCmd(((0xc0u+unsigned(stage)*2+unsigned(alpha))<<24)|inputs);
}
extern "C" void GXSetTevColorIn(GXTevStageID stage,GXTevColorArg a,GXTevColorArg b,GXTevColorArg c,GXTevColorArg d) {
    selectInputs(stage,a,b,c,d,false);
}
extern "C" void GXSetTevAlphaIn(GXTevStageID stage,GXTevAlphaArg a,GXTevAlphaArg b,GXTevAlphaArg c,GXTevAlphaArg d) {
    selectInputs(stage,a,b,c,d,true);
}
static void selectOperation(GXTevStageID stage,GXTevOp op,GXTevBias bias,GXTevScale scale,GXBool clamp,GXTevRegID destination,bool alpha) {
    const unsigned operation=unsigned(op);
    if(unsigned(stage)>15 || (operation>1 && (operation<8 || operation>15)) ||
       unsigned(bias)>2 || unsigned(scale)>3 || unsigned(clamp)>1 || unsigned(destination)>3)
        throw std::invalid_argument("GX TEV operation");
    // Comparison operations encode bias=3 and their width in the scale bits,
    // as in the original SDK GDSetTevColorCalc/AlphaCalcAndSwap writers.
    const unsigned encodedBias=operation<=1 ? unsigned(bias) : 3;
    const unsigned encodedScale=operation<=1 ? unsigned(scale) : ((operation>>1)&3);
    const unsigned fields=(encodedBias<<16)|((operation&1)<<18)|(unsigned(clamp)<<19)|
        (encodedScale<<20)|(unsigned(destination)<<22);
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfeff0000u);
    GDWriteBPCmd(((0xc0u+unsigned(stage)*2+unsigned(alpha))<<24)|fields);
}
extern "C" void GXSetTevColorOp(GXTevStageID stage,GXTevOp op,GXTevBias bias,GXTevScale scale,GXBool clamp,GXTevRegID destination) {
    selectOperation(stage,op,bias,scale,clamp,destination,false);
}
extern "C" void GXSetTevAlphaOp(GXTevStageID stage,GXTevOp op,GXTevBias bias,GXTevScale scale,GXBool clamp,GXTevRegID destination) {
    selectOperation(stage,op,bias,scale,clamp,destination,true);
}

extern "C" void GXSetTevColor(GXTevRegID id,GXColor color) {
    if(unsigned(id)>3)throw std::invalid_argument("GX TEV color register");
    GDOverflowCheck(20);
    J3DGDSetTevColorS10(id,GXColorS10{color.r,color.g,color.b,color.a});
}
extern "C" void GXSetTevColorS10(GXTevRegID id,GXColorS10 color) {
    if(unsigned(id)>3)throw std::invalid_argument("GX TEV color register");
    GDOverflowCheck(20);J3DGDSetTevColorS10(id,color);
}
extern "C" void GXSetTevKColor(GXTevKColorID id,GXColor color) {
    if(unsigned(id)>3)throw std::invalid_argument("GX TEV constant register");
    GDOverflowCheck(10);
    J3DGDSetTevKColor(id,color);
}
static void selectKonst(GXTevStageID stage,unsigned value,bool alpha) {
    if(unsigned(stage)>15 || value>31)throw std::invalid_argument("GX TEV konst selection");
    const unsigned shift=(stage&1 ? 14 : 4)+(alpha ? 5 : 0);
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe000000u | (31u<<shift));
    GDWriteBPCmd(((0xf6u+unsigned(stage)/2)<<24) | (value<<shift));
}
extern "C" void GXSetTevKColorSel(GXTevStageID stage,GXTevKColorSel value) { selectKonst(stage,value,false); }
extern "C" void GXSetTevKAlphaSel(GXTevStageID stage,GXTevKAlphaSel value) { selectKonst(stage,value,true); }
#include <revolution/gx/GXPixel.h>
extern "C" void GXSetBlendMode(GXBlendMode mode,GXBlendFactor source,GXBlendFactor destination,GXLogicOp logic){
    if(unsigned(mode)>3 || unsigned(source)>7 || unsigned(destination)>7 || unsigned(logic)>15)throw std::invalid_argument("GX blend mode");
    const u32 value=(unsigned(mode)&1)|(u32(mode==GX_BM_LOGIC)<<1)|(unsigned(destination)<<5)|(unsigned(source)<<8)|(u32(mode==GX_BM_SUBTRACT)<<11)|(unsigned(logic)<<12);
    GDOverflowCheck(10);GDWriteBPCmd(0xfe00ffe3u);GDWriteBPCmd(0x41000000u|value);
}
extern "C" void GXSetColorUpdate(GXBool enabled){if(unsigned(enabled)>1)throw std::invalid_argument("GX color update");GDOverflowCheck(10);GDWriteBPCmd(0xfe000008u);GDWriteBPCmd(0x41000000u|(u32(enabled)<<3));}
extern "C" void GXSetAlphaUpdate(GXBool enabled){if(unsigned(enabled)>1)throw std::invalid_argument("GX alpha update");GDOverflowCheck(10);GDWriteBPCmd(0xfe000010u);GDWriteBPCmd(0x41000000u|(u32(enabled)<<4));}
extern "C" void GXSetZMode(GXBool compare,GXCompare function,GXBool update){
    if(unsigned(compare)>1 || unsigned(function)>7 || unsigned(update)>1)throw std::invalid_argument("GX depth mode");
    GDOverflowCheck(10);GDWriteBPCmd(0xfe00001fu);GDWriteBPCmd(0x40000000u|u32(compare)|(u32(function)<<1)|(u32(update)<<4));
}
extern "C" void GXSetZCompLoc(GXBool beforeTexture){
    if(unsigned(beforeTexture)>1)throw std::invalid_argument("GX depth comparison location");
    GDOverflowCheck(10);GDWriteBPCmd(0xfe000040u);GDWriteBPCmd(0x43000000u|(u32(beforeTexture)<<6));
}
extern "C" void GXSetFogRangeAdj(GXBool enabled,u16 center,const GXFogAdjTable* table){
    if(unsigned(enabled)>1 || (enabled && !table))throw std::invalid_argument("GX fog range adjustment");
    if(enabled)for(unsigned i=0;i<10;++i)if(table->r[i]>4095)throw std::invalid_argument("GX fog range table field");
    GDOverflowCheck(enabled?30:5);
    if(enabled)for(unsigned i=0;i<10;i+=2)GDWriteBPCmd(((0xe9u+i/2)<<24)|u32(table->r[i])|(u32(table->r[i+1])<<12));
    GDWriteBPCmd(0xe8000000u|((u32(center)+342)&1023u)|(u32(enabled)<<10));
}
#include <cmath>
#include <cstring>
namespace {
u32 nativeFogFloat(float value){if(!std::isfinite(value))throw std::invalid_argument("Nonfinite GX fog coefficient");u32 bits;std::memcpy(&bits,&value,4);return ((bits>>12)&2047)|(((bits>>23)&255)<<11)|((bits>>31)<<19);}
}
extern "C" void GXSetFog(GXFogType type,f32 start,f32 end,f32 nearValue,f32 farValue,GXColor color){
    unsigned mode=unsigned(type),function=mode&7;bool orthographic=bool(mode&8);
    if(mode>15 || !(mode==0 || function==2 || function>=4) || !std::isfinite(start) || !std::isfinite(end) || !std::isfinite(nearValue) || !std::isfinite(farValue))throw std::invalid_argument("GX fog parameters");
    float a=0,c=0;u32 magnitude=0,shift=0;
    bool degenerate=farValue==nearValue || end==start;
    if(orthographic){if(!degenerate){float reciprocal=1.f/(end-start);a=(farValue-nearValue)*reciprocal;c=(start-nearValue)*reciprocal;}}
    else {float A=0,B=.5f,C=0;if(!degenerate){A=(farValue*nearValue)/((farValue-nearValue)*(end-start));B=farValue/(farValue-nearValue);C=start/(end-start);}if(!std::isfinite(B) || B<0)throw std::invalid_argument("GX fog normalization domain");int exponent=0;while(B>1){B/=2;++exponent;}while(B>0 && B<.5f){B*=2;--exponent;}a=std::ldexp(A,-(exponent+1));c=C;magnitude=u32(B*8388638.f)&0xffffffu;shift=u32(exponent+1)&31;}
    u32 packedA=nativeFogFloat(a),packedC=nativeFogFloat(c)|(u32(orthographic)<<20)|(function<<21);
    GDOverflowCheck(25);GDWriteBPCmd(0xee000000u|packedA);GDWriteBPCmd(0xef000000u|magnitude);GDWriteBPCmd(0xf0000000u|shift);GDWriteBPCmd(0xf1000000u|packedC);GDWriteBPCmd(0xf2000000u|(u32(color.r)<<16)|(u32(color.g)<<8)|color.b);
}
