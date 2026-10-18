#include <revolution/gx/GXBump.h>
#include <revolution/gd/GDBase.h>
#include <revolution/gd/GDIndirect.h>
#include <stdexcept>
#include <cstdio>

extern "C" void GXSetTevIndirect(GXTevStageID stage,GXIndTexStageID indirect,GXIndTexFormat format,
    GXIndTexBiasSel bias,GXIndTexMtxID matrix,GXIndTexWrap wrapS,GXIndTexWrap wrapT,
    GXBool addPrevious,GXBool unmodifiedLod,GXIndTexAlphaSel alpha) {
    const unsigned m=unsigned(matrix);
    const bool validMatrix=m<=3 || (m>=5 && m<=7) || (m>=9 && m<=11);
    if(unsigned(stage)>15 || unsigned(indirect)>3 || unsigned(format)>3 || unsigned(bias)>7 ||
       !validMatrix || unsigned(wrapS)>6 || unsigned(wrapT)>6 ||
       unsigned(addPrevious)>1 || unsigned(unmodifiedLod)>1 || unsigned(alpha)>3)
        throw std::invalid_argument("GX indirect TEV configuration");
    GDOverflowCheck(5);
    GDWriteBPCmd(IND_CMD(indirect,format,bias,alpha,matrix,wrapS,wrapT,unmodifiedLod,addPrevious,0x10+unsigned(stage)));
}

extern "C" void GXSetIndTexOrder(GXIndTexStageID stage,GXTexCoordID coord,GXTexMapID map) {
    if(stage>GX_INDTEXSTAGE3)throw std::invalid_argument("GX indirect stage");
    if(map==GX_TEXMAP_NULL)map=GX_TEXMAP0;
    if(coord==GX_TEXCOORD_NULL)coord=GX_TEXCOORD0;
    if(map>GX_TEXMAP7 || coord>GX_TEXCOORD7)throw std::invalid_argument("GX indirect texture source");
    const unsigned shift=unsigned(stage)*6;
    // Partial BP update preserves other stage fields in the command-side state.
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe000000u | (63u<<shift));
    GDWriteBPCmd(0x27000000u | ((unsigned(map)|(unsigned(coord)<<3))<<shift));
}
extern "C" void GXSetTevDirect(GXTevStageID stage) {
    if(unsigned(stage)>15)throw std::invalid_argument("GX TEV stage");
    // Original GXSetTevDirect selects zero-valued indirect fields: no bias,
    // matrix, wrap, previous-stage addition, indirect alpha or indirect LOD.
    GDOverflowCheck(5);
    GDWriteBPCmd((0x10u+unsigned(stage))<<24);
}
extern "C" void GXSetNumIndStages(u8 count){
    if(count>4)throw std::invalid_argument("GX indirect stage count");
    // Original SDK general-mode NBMP field; native command state applies immediately.
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe070000u);
    GDWriteBPCmd(u32(count)<<16);
}
#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXLighting.h>
extern "C" void GXSetNumTexGens(u8 count){
    if(count>8)throw std::invalid_argument("GX texture generator count");
    GDOverflowCheck(19);
    GDWriteBPCmd(0xfe00000fu);GDWriteBPCmd(count);
    GDWriteXFCmd(0x103f,count);
}
extern "C" void GXSetNumChans(u8 count){
    if(count>2)throw std::invalid_argument("GX color channel count");
    GDOverflowCheck(19);
    GDWriteBPCmd(0xfe000070u);GDWriteBPCmd(u32(count)<<4);
    GDWriteXFCmd(0x1009,count);
}
extern "C" void GXSetChanCtrl(GXChannelID channel,GXBool enabled,GXColorSrc ambient,GXColorSrc material,u32 lights,GXDiffuseFn diffuse,GXAttnFn attenuation){
    if(unsigned(channel)>5 || unsigned(enabled)>1 || unsigned(ambient)>1 || unsigned(material)>1 || unsigned(diffuse)>2 || unsigned(attenuation)>2) {
        char detail[320];
        std::snprintf(detail, sizeof(detail), "GX channel lighting control: channel=%u enabled=%u ambient=%u material=%u lights=%08X diffuse=%u attenuation=%u caller=%p", unsigned(channel), unsigned(enabled), unsigned(ambient), unsigned(material), unsigned(lights), unsigned(diffuse), unsigned(attenuation), __builtin_return_address(0));
        throw std::invalid_argument(detail);
    }
    const u32 value=unsigned(material)|(unsigned(enabled)<<1)|((lights&15)<<2)|(unsigned(ambient)<<6)|
        ((attenuation==GX_AF_SPEC?0u:unsigned(diffuse))<<7)|(u32(attenuation!=GX_AF_NONE)<<9)|
        (u32(attenuation!=GX_AF_SPEC)<<10)|(((lights>>4)&15)<<11);
    const unsigned index=unsigned(channel)&3;const bool combined=unsigned(channel)>=4;
    GDOverflowCheck(combined?18:9);
    GDWriteXFCmd(0x100e + index,value);
    if(combined)GDWriteXFCmd(0x1010+index,value);
}

#include <revolution/gx/GXTransform.h>
extern "C" void GXLoadTexMtxImm(const f32 matrix[][4],u32 id,GXTexMtxType type){
    if(!matrix || unsigned(type)>1 || (id>60 && (id<64 || id>125)))throw std::invalid_argument("GX texture matrix upload");
    const unsigned count=type==GX_MTX2x4?8:12;
    const unsigned address=id>=64?0x500+((id-64)<<2):id<<2;
    GDOverflowCheck(5+count*4);
    GDWriteXFCmdHdr(u16(address),u8(count));
    for(unsigned i=0;i<count;++i)GDWrite_f32(matrix[i/4][i%4]);
}
namespace {
u32 nativeAmbientColors[2]={0,0},nativeMaterialColors[2]={0,0};
void nativeChannelColor(GXChannelID channel,GXColor color,u32* retained,u16 base){
    unsigned id=unsigned(channel);
    if(id>5)throw std::invalid_argument("GX channel color identifier");
    unsigned index=id&1;
    u32 packed=(u32(color.r)<<24)|(u32(color.g)<<16)|(u32(color.b)<<8)|color.a;
    u32 next=id<2 ? ((packed&0xffffff00u)|(retained[index]&255u)) :
        id<4 ? ((retained[index]&0xffffff00u)|color.a) : packed;
    GDOverflowCheck(9);
    GDWriteXFCmd(base+index,next);
    retained[index]=next;
}
}
extern "C" void GXSetChanAmbColor(GXChannelID channel,GXColor color){nativeChannelColor(channel,color,nativeAmbientColors,0x100a);}
extern "C" void GXSetChanMatColor(GXChannelID channel,GXColor color){nativeChannelColor(channel,color,nativeMaterialColors,0x100c);}
extern "C" void GXSetClipMode(GXClipMode mode){
    if(unsigned(mode)>1)throw std::invalid_argument("GX clip mode");
    GDOverflowCheck(9);GDWriteXFCmd(0x1005,u32(mode));
}
namespace {u32 nativeMatrixIndices[2]={0,0};}
extern "C" void GXSetTexCoordGen2(GXTexCoordID destination,GXTexGenType function,GXTexGenSrc source,u32 matrix,GXBool normalize,u32 postMatrix){
    unsigned dst=unsigned(destination),fn=unsigned(function),src=unsigned(source);
    bool bump=fn>=unsigned(GX_TG_BUMP0) && fn<=unsigned(GX_TG_BUMP7),srtg=function==GX_TG_SRTG;
    if(dst>7 || fn>unsigned(GX_TG_SRTG) || src>unsigned(GX_TG_COLOR1) || matrix>GX_IDENTITY || matrix%3 || unsigned(normalize)>1 || postMatrix<GX_PTTEXMTX0 || postMatrix>GX_PTIDENTITY || (postMatrix!=GX_PTIDENTITY && (postMatrix-GX_PTTEXMTX0)%3) || (bump && (src<unsigned(GX_TG_TEXCOORD0) || src>unsigned(GX_TG_TEXCOORD6))) || (srtg && source!=GX_TG_COLOR0 && source!=GX_TG_COLOR1))throw std::invalid_argument("GX texture coordinate generator");
    unsigned row=5,form=0;
    if(source==GX_TG_POS){row=0;form=1;}else if(source==GX_TG_NRM){row=1;form=1;}else if(source==GX_TG_BINRM){row=3;form=1;}else if(source==GX_TG_TANGENT){row=4;form=1;}else if(source==GX_TG_COLOR0 || source==GX_TG_COLOR1)row=2;else if(src>=unsigned(GX_TG_TEX0) && src<=unsigned(GX_TG_TEX7))row=5+src-unsigned(GX_TG_TEX0);
    u32 control=(u32(function==GX_TG_MTX3x4)<<1)|(form<<2)|(row<<7);
    if(bump)control|=(1u<<4)|((src-unsigned(GX_TG_TEXCOORD0))<<12)|((fn-unsigned(GX_TG_BUMP0))<<15);
    if(srtg)control=(2u<<7)|((source==GX_TG_COLOR0?2u:3u)<<4);
    unsigned bank=dst/4,shift=bank?(dst%4)*6:6+dst*6;
    u32 indices=(nativeMatrixIndices[bank]&~(63u<<shift))|(matrix<<shift);
    GDOverflowCheck(33);GDWriteXFCmd(0x1040+dst,control);GDWriteXFCmd(0x1050+dst,(postMatrix-GX_PTTEXMTX0)|(u32(normalize)<<8));GDWriteCPCmd(bank?0x40:0x30,indices);GDWriteXFCmd(0x1018+bank,indices);nativeMatrixIndices[bank]=indices;
}

extern "C" void GXSetCurrentMtx(u32 id){
    if(id>27 || id%3)throw std::invalid_argument("GX position matrix index");
    u32 next=(nativeMatrixIndices[0]&~63u)|id;
    GDOverflowCheck(15);GDWriteCPCmd(0x30,next);GDWriteXFCmd(0x1018,next);nativeMatrixIndices[0]=next;
}
#include <revolution/gx/GXTransform.h>
#include <cmath>
extern "C" void GXLoadPosMtxImm(const f32 matrix[3][4],u32 id){
    if(!matrix || id>27 || id%3)throw std::invalid_argument("GX position matrix upload");
    for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col)if(!std::isfinite(matrix[row][col]))throw std::invalid_argument("Nonfinite GX position matrix");
    GDOverflowCheck(53);GDWriteXFCmdHdr(u16(id*4),12);
    for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col)GDWrite_f32(matrix[row][col]);
}

#include <cmath>
extern "C" void GXSetIndTexMtx(GXIndTexMtxID matrix,const f32 coefficients[2][3],s8 exponent){
 unsigned value=unsigned(matrix),index;
 if(value>=1 && value<=3)index=value-1;else if(value>=5 && value<=7)index=value-5;else if(value>=9 && value<=11)index=value-9;else throw std::invalid_argument("Invalid indirect matrix");
 if(!coefficients || exponent<-17 || exponent>46)throw std::invalid_argument("Invalid indirect matrix exponent");
 u32 words[3];unsigned encoded=unsigned(int(exponent)+17);
 for(unsigned column=0;column<3;++column){u32 terms[2];for(unsigned row=0;row<2;++row){float scaled=coefficients[row][column]*1024.0f;if(!std::isfinite(scaled) || double(scaled)<-2147483648.0 || double(scaled)>=2147483648.0)throw std::invalid_argument("Invalid indirect matrix coefficient");terms[row]=u32(s32(scaled))&2047;}words[column]=((6+index*3+column)<<24)|terms[0]|(terms[1]<<11)|(((encoded>>(column*2))&3)<<22);}
 GDOverflowCheck(20);GDWriteBPCmd(0xfeffffff);for(auto word:words)GDWriteBPCmd(word);
}
extern "C" void GXSetIndTexCoordScale(GXIndTexStageID stage,GXIndTexScale scaleS,GXIndTexScale scaleT){
 if(unsigned(stage)>3 || unsigned(scaleS)>8 || unsigned(scaleT)>8)throw std::invalid_argument("Invalid indirect coordinate scale");
 unsigned shift=(unsigned(stage)&1)*8;u32 mask=255u<<shift;
 GDOverflowCheck(10);GDWriteBPCmd(0xfe000000|mask);GDWriteBPCmd(((0x25+unsigned(stage)/2)<<24)|((unsigned(scaleS)|(unsigned(scaleT)<<4))<<shift));
}
extern "C" void GXLoadNrmMtxImm(const f32 matrix[3][4],u32 id){
 if(!matrix || id>27 || id%3)throw std::invalid_argument("GX normal matrix upload");
 for(unsigned r=0;r<3;++r)for(unsigned c=0;c<3;++c)if(!std::isfinite(matrix[r][c]))throw std::invalid_argument("Nonfinite GX normal matrix");
 GDOverflowCheck(41);GDWriteXFCmdHdr(u16(0x400+id*3),9);
 for(unsigned r=0;r<3;++r)for(unsigned c=0;c<3;++c)GDWrite_f32(matrix[r][c]);
}
extern "C" void GXLoadTexMtxIndx(u16 index,u32 id,GXTexMtxType type){
    if(unsigned(type)>1 || (id>60 && (id<64 || id>125)))
        throw std::invalid_argument("GX indexed texture matrix upload");
    const u32 count=type==GX_MTX2x4?8:12;
    const u32 address=id>=64?0x500+((id-64)<<2):id<<2;
    // Original GXTransform indexed C command: 16-bit source index,
    // four-bit word count minus one and twelve-bit XF destination.
    GDOverflowCheck(5);
    GDWrite_u8(0x30);
    GDWrite_u32((u32(index)<<16)|((count-1)<<12)|address);
}
extern "C" void GXSetTevIndWarp(GXTevStageID stage,GXIndTexStageID indirect,GXBool signedOffset,GXBool replaceMode,GXIndTexMtxID matrix){
    if(unsigned(signedOffset)>1 || unsigned(replaceMode)>1)
        throw std::invalid_argument("GX indirect warp flags");
    const auto wrap=replaceMode?GX_ITW_0:GX_ITW_OFF;
    GXSetTevIndirect(stage,indirect,GX_ITF_8,signedOffset?GX_ITB_STU:GX_ITB_NONE,
        matrix,wrap,wrap,GX_FALSE,GX_FALSE,GX_ITBA_OFF);
}
extern "C" void GXEnableTexOffsets(GXTexCoordID coord,GXBool line,GXBool point){
    if(unsigned(coord)>7 || unsigned(line)>1 || unsigned(point)>1)
        throw std::invalid_argument("GX texture offset flags");
    // Masked SU update preserves texture size and unrelated wrap fields.
    GDOverflowCheck(10);
    GDWriteBPCmd(0xfe0c0000);
    GDWriteBPCmd(((0x30+unsigned(coord)*2)<<24)|(u32(line)<<18)|(u32(point)<<19));
}
