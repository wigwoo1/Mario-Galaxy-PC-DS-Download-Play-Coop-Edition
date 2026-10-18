#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/gd/GDBase.h>
#include <stdexcept>
extern "C" void GXBegin(GXPrimitive primitive,GXVtxFmt format,u16 vertices){
    const unsigned p=unsigned(primitive);
    if(unsigned(format)>7 || !(p==0x80 || p==0x90 || p==0x98 || p==0xa0 || p==0xa8 || p==0xb0 || p==0xb8))throw std::invalid_argument("GX primitive header");
    GDOverflowCheck(3);GDWrite_u8(u8(p|unsigned(format)));GDWrite_u16(vertices);
}
namespace {
u32 descriptorLow=1u<<9,descriptorHigh=0;unsigned descriptorNormal=0;
void emitDescriptor(u32 low,u32 high,unsigned normal){
    unsigned colors=(low&(3u<<15))?2:((low&(3u<<13))?1:0),textures=0;
    for(unsigned i=0;i<8;++i)if((high>>(i*2))&3)textures=i+1;
    GDOverflowCheck(21);GDWriteCPCmd(0x50,low);GDWriteCPCmd(0x60,high);GDWriteXFCmd(0x1008,colors|(normal<<2)|(textures<<4));
    descriptorLow=low;descriptorHigh=high;descriptorNormal=normal;
}
}
extern "C" void GXClearVtxDesc(){emitDescriptor(1u<<9,0,0);}
extern "C" void GXSetVtxDesc(GXAttr attribute,GXAttrType type){
    unsigned id=unsigned(attribute),value=unsigned(type);
    if(value>3 || !((id<=unsigned(GX_VA_TEX7)) || attribute==GX_VA_NBT) || (id<=8 && value>1))throw std::invalid_argument("GX vertex descriptor");
    u32 low=descriptorLow,high=descriptorHigh;unsigned normal=descriptorNormal;
    if(id<=8)low=(low&~(1u<<id))|(value<<id);
    else if(attribute==GX_VA_POS)low=(low&~(3u<<9))|(value<<9);
    else if(attribute==GX_VA_NRM || attribute==GX_VA_NBT){if(value){low=(low&~(3u<<11))|(value<<11);normal=attribute==GX_VA_NBT?2:1;}else if(normal==(attribute==GX_VA_NBT?2u:1u)){low&=~(3u<<11);normal=0;}}
    else if(attribute==GX_VA_CLR0 || attribute==GX_VA_CLR1){unsigned shift=13+2*(id-unsigned(GX_VA_CLR0));low=(low&~(3u<<shift))|(value<<shift);}
    else {unsigned shift=2*(id-unsigned(GX_VA_TEX0));high=(high&~(3u<<shift))|(value<<shift);}
    emitDescriptor(low,high,normal);
}
namespace {
void nativeVatField(u32& word,u32 value,unsigned shift,unsigned width){u32 mask=((1u<<width)-1)<<shift;word=(word&~mask)|((value<<shift)&mask);}
#include "generated/VatFieldKernel.inc"
u32 vertexFormats[8][3]={};
}
extern "C" void GXSetVtxAttrFmt(GXVtxFmt format,GXAttr attribute,GXCompCnt count,GXCompType type,u8 fraction){
    unsigned id=unsigned(attribute),c=unsigned(count),t=unsigned(type),slot=unsigned(format);
    bool normal=attribute==GX_VA_NRM || attribute==GX_VA_NBT,color=attribute==GX_VA_CLR0 || attribute==GX_VA_CLR1;
    if(slot>7 || fraction>31 || !(attribute==GX_VA_POS || normal || color || (id>=unsigned(GX_VA_TEX0) && id<=unsigned(GX_VA_TEX7))) || c>(normal?2u:1u) || t>(color?5u:4u) || (normal && !(t==1 || t==3 || t==4)))throw std::invalid_argument("GX vertex attribute format");
    u32 a=vertexFormats[slot][0],b=vertexFormats[slot][1],d=vertexFormats[slot][2];SETVAT(&a,&b,&d,attribute,count,type,fraction);
    GDOverflowCheck(18);GDWriteCPCmd(0x70+slot,a);GDWriteCPCmd(0x80+slot,b);GDWriteCPCmd(0x90+slot,d);
    vertexFormats[slot][0]=a;vertexFormats[slot][1]=b;vertexFormats[slot][2]=d;
}
extern "C" void GXSetVtxAttrFmtv(GXVtxFmt format,const GXVtxAttrFmtList* list){
    if(!list)throw std::invalid_argument("Null GX vertex attribute format list");
    // The native command stream applies register updates immediately, so the
    // vector entry point is expressed through the scalar emitter. Keep a hard
    // bound in case a malformed list omits the SDK sentinel.
    for(unsigned i=0;i<32;++i){
        if(list[i].attr==GX_VA_NULL)return;
        GXSetVtxAttrFmt(format,list[i].attr,list[i].cnt,list[i].type,list[i].frac);
    }
    throw std::invalid_argument("Unterminated GX vertex attribute format list");
}
extern "C" void GXSetCoPlanar(GXBool enabled){
 if(unsigned(enabled)>1)throw std::invalid_argument("Invalid coplanar flag");
 GDOverflowCheck(10);GDWriteBPCmd(0xfe080000);GDWriteBPCmd(u32(enabled)<<19);
}
// Native command streams apply these fields immediately rather than retaining
// the console SDK's deferred register cache. Masks preserve unrelated state.
extern "C" void GXSetCullMode(GXCullMode mode){
 const unsigned value=unsigned(mode);
 if(value>3)throw std::invalid_argument("Invalid cull mode");
 const u32 hardware=((value&1)<<1)|((value&2)>>1);
 GDOverflowCheck(10);GDWriteBPCmd(0xfe00c000);GDWriteBPCmd(hardware<<14);
}
extern "C" void GXSetDither(GXBool enabled){
 if(unsigned(enabled)>1)throw std::invalid_argument("Invalid dither flag");
 GDOverflowCheck(10);GDWriteBPCmd(0xfe000004);GDWriteBPCmd(0x41000000|(u32(enabled)<<2));
}
extern "C" void GXSetLineWidth(u8 width,GXTexOffset offset){
 if(unsigned(offset)>5)throw std::invalid_argument("GX line texture offset");
 GDOverflowCheck(10);GDWriteBPCmd(0xfe0700ff);GDWriteBPCmd(0x22000000|u32(width)|(u32(offset)<<16));
}
extern "C" void GXSetPointSize(u8 size,GXTexOffset offset){
 if(unsigned(offset)>5)throw std::invalid_argument("GX point texture offset");
 GDOverflowCheck(10);GDWriteBPCmd(0xfe38ff00);GDWriteBPCmd(0x22000000|(u32(size)<<8)|(u32(offset)<<19));
}
extern "C" void GXSetPixelFmt(GXPixelFmt format,GXZFmt16 depth){
 const unsigned f=unsigned(format),z=unsigned(depth);
 if(f>7 || z>3)throw std::invalid_argument("GX pixel/depth format");
 static constexpr u32 hardware[]={0,1,2,3,4,4,4,5};
 // Original PE_CONTROL and GEN_MODE fields, applied immediately on native.
 GDOverflowCheck(hardware[f]==4?30:20);
 GDWriteBPCmd(0xfe00003f);GDWriteBPCmd(0x43000000|hardware[f]|(z<<3));
 GDWriteBPCmd(0xfe000200);GDWriteBPCmd(f==2?0x00000200:0);
 if(hardware[f]==4){GDWriteBPCmd(0xfe000600);GDWriteBPCmd(0x42000000|((f-4)<<9));}
}
