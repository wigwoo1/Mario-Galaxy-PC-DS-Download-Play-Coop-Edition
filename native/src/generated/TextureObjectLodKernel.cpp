#include <revolution/gx.h>
#include <revolution/gx/GXRegs.h>
#include <private/ra_gen.h>
#include <revolution/gx/shortcut_tx_reg.h>
#undef FAST_GPFLAGSET
#define FAST_GPFLAGSET(regOrg,newFlag,regName) ((regOrg)=((regOrg)&~regName##_MASK)|((u32(newFlag)<<regName##_SHIFT)&regName##_MASK))
#include <cmath>
#include <cstring>
#include <stdexcept>
struct NativeTextureObject {u32 mode0,mode1,image0,image3,userData,fmt,tlutName;u16 loadCnt;u8 loadFmt,flags;};
static_assert(sizeof(NativeTextureObject)==sizeof(GXTexObj));
static u8 GX2HWFiltConv[]={0,4,1,5,2,6};
#include <Native/GpuResources.hpp>
static void nativeTextureTileShift(GXTexFmt fmt, u32* rowTileS, u32* colTileS) {
    switch (fmt) {
    case GX_TF_C4:
    case GX_TF_I4:
    case GX_TF_CMPR:
    case GX_CTF_R4:
    case GX_CTF_Z4:
        *rowTileS = 3;
        *colTileS = 3;
        break;

    case GX_TF_C8:
    case GX_TF_I8:
    case GX_TF_IA4:
    case GX_TF_A8:
    case GX_TF_Z8:
    case GX_CTF_RA4:
    case GX_CTF_R8:
    case GX_CTF_G8:
    case GX_CTF_B8:
    case GX_CTF_Z8M:
    case GX_CTF_Z8L:
        *rowTileS = 3;
        *colTileS = 2;
        break;

    // 16-bit+ formats:
    case GX_TF_C14X2:
    case GX_TF_IA8:
    case GX_TF_RGB565:
    case GX_TF_RGB5A3:
    case GX_TF_RGBA8:
    case GX_TF_Z16:
    case GX_TF_Z24X8:
    case GX_CTF_RA8:
    case GX_CTF_RG8:
    case GX_CTF_GB8:
    case GX_CTF_Z16L:
        *rowTileS = 2;
        *colTileS = 2;
        break;
    default:
        *rowTileS = *colTileS = 0;
        break;
    }
    return;
}

static u32 nativeTextureBufferSize(u16 width, u16 height, u32 format, GXBool mipmap, u8 max_lod) {
    u32 tileShiftX, tileShiftY, tileBytes;
    u32 bufferSize, nx, ny, level;

    nativeTextureTileShift((GXTexFmt)format, &tileShiftX, &tileShiftY);

    if (format == GX_TF_RGBA8 || format == GX_TF_Z24X8) {
        tileBytes = 64;
    } else {
        tileBytes = 32;
    }

    if (mipmap == GX_TRUE) {
        bufferSize = 0;

        for (level = 0; level < max_lod; level++) {
            nx = (u32)((width + (1 << tileShiftX) - 1) >> tileShiftX);
            ny = (u32)((height + (1 << tileShiftY) - 1) >> tileShiftY);

            bufferSize += nx * ny * tileBytes;

            if (width == 1 && height == 1)
                break;

            width = (u16)((width > 1) ? (width >> 1) : 1);
            height = (u16)((height > 1) ? (height >> 1) : 1);
        }
    } else {
        nx = (u32)((width + (1 << tileShiftX) - 1) >> tileShiftX);
        ny = (u32)((height + (1 << tileShiftY) - 1) >> tileShiftY);

        bufferSize = nx * ny * tileBytes;
    }

    return bufferSize;
}


#define IsCITexFmt(fmt) (unsigned(fmt)==8 || unsigned(fmt)==9 || unsigned(fmt)==10)
static size_t nativeTextureBytes(unsigned w,unsigned h,GXTexFmt format,bool mipmap){
 switch(format){
 case GX_TF_I4:case GX_TF_I8:case GX_TF_IA4:case GX_TF_IA8:case GX_TF_RGB565:case GX_TF_RGB5A3:case GX_TF_RGBA8:case GX_TF_CMPR:
 case GX_TF_C4:case GX_TF_C8:case GX_TF_C14X2:case GX_TF_Z8:case GX_TF_Z16:case GX_TF_Z24X8:
 case GX_CTF_R4:case GX_CTF_RA4:case GX_CTF_RA8:case GX_CTF_YUVA8:case GX_CTF_A8:case GX_CTF_R8:case GX_CTF_G8:case GX_CTF_B8:case GX_CTF_RG8:case GX_CTF_GB8:
 case GX_CTF_Z4:case GX_CTF_Z8M:case GX_CTF_Z8L:case GX_CTF_Z16L:break;
 default:throw std::invalid_argument("Unsupported native texture format");
 }
 // Share the original SDK's tile-size rules, including Z and copy-texture
 // formats, instead of maintaining a second block-layout table here.
 return nativeTextureBufferSize(u16(w),u16(h),u32(format),mipmap?GX_TRUE:GX_FALSE,mipmap?0xff:1);
}
void GXInitTexObj(GXTexObj* obj, void* image_ptr, u16 width, u16 height, GXTexFmt format, GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, GXBool mipmap) {
    u32 imageBase;
    u32 maxLOD;
    u16 rowT, colT;
    u32 rowC, colC;

    if(!obj || !image_ptr || !width || !height || width>1024 || height>1024 || unsigned(wrap_s)>2 || unsigned(wrap_t)>2 || unsigned(mipmap)>1)throw std::invalid_argument("Invalid native texture object"); auto nativeAddress=Native::gpuResources().address(image_ptr); Native::gpuResources().resolve(nativeAddress,nativeTextureBytes(width,height,format,mipmap)); NativeTextureObject local{}; auto* t=&local;

    

    SC_TX_SETMODE0_SET_WRAP_S(t->mode0, wrap_s);
    SC_TX_SETMODE0_SET_WRAP_T(t->mode0, wrap_t);
    SC_TX_SETMODE0_SET_MAG_FILTER(t->mode0, 1);

    if (mipmap) {
        u8 lmax;

        t->flags |= 1;
        if (IsCITexFmt(format)) {
            SC_TX_SETMODE0_SET_MIN_FILTER(t->mode0, 5);
        } else {
            SC_TX_SETMODE0_SET_MIN_FILTER(t->mode0, 6);
        }

        if (width > height) {
            maxLOD = (u32)(31 - __builtin_clz(unsigned(width)));
        } else {
            maxLOD = (u32)(31 - __builtin_clz(unsigned(height)));
        }
        lmax = (u8)(maxLOD * 16.0f);
        SC_TX_SETMODE1_SET_MAXLOD(t->mode1, lmax);
    } else {
        SC_TX_SETMODE0_SET_MIN_FILTER(t->mode0, 4);
    }

    t->fmt = format;
    SC_TX_SETIMAGE0_SET_IMAGE_WIDTH(t->image0, (width - 1));
    SC_TX_SETIMAGE0_SET_IMAGE_HEIGHT(t->image0, (height - 1));
    SC_TX_SETIMAGE0_SET_FORMAT(t->image0, (format & 0xf));

    imageBase = nativeAddress >> 5;
    SC_TX_SETIMAGE3_SET_IMAGE_BASE(t->image3, imageBase);

    switch (format & 0xf) {
    case GX_TF_I4:
    case GX_TF_C4:
        t->loadFmt = 1;
        rowT = 3;
        colT = 3;
        break;

    case GX_TF_I8:
    case GX_TF_IA4:
    case GX_TF_C8:
        t->loadFmt = 2;
        rowT = 3;
        colT = 2;
        break;

    case GX_TF_IA8:
    case GX_TF_C14X2:
    case GX_TF_RGB565:
    case GX_TF_RGB5A3:
        t->loadFmt = 2;
        rowT = 2;
        colT = 2;
        break;

    case GX_TF_RGBA8:
        t->loadFmt = 3;
        rowT = 2;
        colT = 2;
        break;

    case GX_TF_CMPR:
        t->loadFmt = 0;
        rowT = 3;
        colT = 3;
        break;

    default:
        t->loadFmt = 2;
        rowT = 2;
        colT = 2;
        break;
    }

    rowC = (u32)((width + (1 << rowT) - 1) >> rowT);
    colC = (u32)((height + (1 << colT) - 1) >> colT);
    t->loadCnt = (u16)((rowC * colC) & TX_LOADBLOCK3_COUNT_MASK);
    t->flags |= 2; std::memcpy(obj,&local,sizeof(local));
}


void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter min_filt, GXTexFilter mag_filt, f32 min_lod, f32 max_lod, f32 lod_bias, GXBool bias_clamp,
                     GXBool do_edge_lod, GXAnisotropy max_aniso) {
    u8 lbias;
    u8 lmin, lmax;
    if(!obj || unsigned(min_filt)>5 || unsigned(mag_filt)>1 || unsigned(max_aniso)>2 || !std::isfinite(min_lod) || !std::isfinite(max_lod) || !std::isfinite(lod_bias))throw std::invalid_argument("Invalid texture LOD arguments"); NativeTextureObject local; std::memcpy(&local,obj,sizeof(local)); auto* t=&local;

    if (lod_bias < -4.0f)
        lod_bias = -4.0f;
    else if (lod_bias >= 4.0f)
        lod_bias = 3.99f;
    lbias = (u8)((s8)(lod_bias * 32.0f));

    SC_TX_SETMODE0_SET_LODBIAS(t->mode0, lbias);

    SC_TX_SETMODE0_SET_MAG_FILTER(t->mode0, ((mag_filt == GX_LINEAR) ? 1 : 0));
    SC_TX_SETMODE0_SET_MIN_FILTER(t->mode0, GX2HWFiltConv[min_filt]);

    SC_TX_SETMODE0_SET_DIAGLOD_ENABLE(t->mode0, ((do_edge_lod) ? 0 : 1));
    SC_TX_SETMODE0_SET_ROUND(t->mode0, 0);
    SC_TX_SETMODE0_SET_FIELD_PREDICT(t->mode0, 0);
    SC_TX_SETMODE0_SET_MAXANISO(t->mode0, max_aniso);
    SC_TX_SETMODE0_SET_LODCLAMP(t->mode0, bias_clamp);

    if (min_lod < 0)
        min_lod = 0;
    else if (min_lod > 10.0f)
        min_lod = 10.0f;
    lmin = (u8)(min_lod * 16.0f);

    if (max_lod < 0)
        max_lod = 0;
    else if (max_lod > 10.0f)
        max_lod = 10.0f;
    lmax = (u8)(max_lod * 16.0f);

    SC_TX_SETMODE1_SET_MINLOD(t->mode1, lmin);
    SC_TX_SETMODE1_SET_MAXLOD(t->mode1, lmax); std::memcpy(obj,&local,sizeof(local));
}


#include <revolution/gd/GDBase.h>
u32 nativePaletteBinding(u32);
extern "C" void GXLoadTexObj(const GXTexObj* object,GXTexMapID slot){
 if(!object || unsigned(slot)>7)throw std::invalid_argument("Invalid texture load slot/object");
 NativeTextureObject value;std::memcpy(&value,object,sizeof(value));
 u32 paletteWord=0;if(!(value.flags&2))paletteWord=nativePaletteBinding(value.tlutName);
 auto width=(value.image0&1023)+1,height=((value.image0>>10)&1023)+1;
 Native::gpuResources().resolve((value.image3&0xffffff)<<5,nativeTextureBytes(width,height,GXTexFmt(value.fmt),bool(value.flags&1)));
 unsigned bank=unsigned(slot)<4?unsigned(slot):unsigned(slot)+28;
 GDOverflowCheck((value.flags&2)?25:30);
 GDWriteBPCmd(0xfeffffff);
 GDWriteBPCmd(((0x80+bank)<<24)|(value.mode0&0xffffff));
 GDWriteBPCmd(((0x84+bank)<<24)|(value.mode1&0xffffff));
 GDWriteBPCmd(((0x88+bank)<<24)|(value.image0&0xffffff));
 GDWriteBPCmd(((0x94+bank)<<24)|(value.image3&0xffffff)); if(!(value.flags&2))GDWriteBPCmd(((0x98+bank)<<24)|(paletteWord&0xffffff));
}
static u8 HW2GXFiltConv[]={0,2,4,0,1,3,5,0};
extern "C" GXTexFmt GXGetTexObjFmt(const GXTexObj* obj){if(!obj)throw std::invalid_argument("Null texture object");NativeTextureObject local;std::memcpy(&local,obj,sizeof(local));return GXTexFmt(local.fmt);}
extern "C" GXBool GXGetTexObjMipMap(const GXTexObj* obj){if(!obj)throw std::invalid_argument("Null texture object");NativeTextureObject local;std::memcpy(&local,obj,sizeof(local));return GXBool((local.flags&1)!=0);}
void GXInitTexObjTlut(GXTexObj* obj, u32 tlut_name) {
    if(!obj)throw std::invalid_argument("Null texture object"); NativeTextureObject local;std::memcpy(&local,obj,sizeof(local));auto* t=&local;
    t->tlutName = tlut_name;std::memcpy(obj,&local,sizeof(local));
}
void GXGetTexObjAll(const GXTexObj* obj, void** image_ptr, u16* width, u16* height, GXTexFmt* format, GXTexWrapMode* wrap_s, GXTexWrapMode* wrap_t,
                    GXBool* mipmap) {
    if(!obj)throw std::invalid_argument("Null texture object"); NativeTextureObject local;std::memcpy(&local,obj,sizeof(local));auto* t=&local;

    *image_ptr = const_cast<u8*>(Native::gpuResources().hostAddress((TX_SETIMAGE3_GET_IMAGE_BASE(t->image3)) << 5,1));
    *width = (u16)(TX_SETIMAGE0_GET_IMAGE_WIDTH(t->image0) + 1);
    *height = (u16)(TX_SETIMAGE0_GET_IMAGE_HEIGHT(t->image0) + 1);
    *format = GXTexFmt(t->fmt);
    *wrap_s = (GXTexWrapMode)TX_SETMODE0_GET_WRAP_S(t->mode0);
    *wrap_t = (GXTexWrapMode)TX_SETMODE0_GET_WRAP_T(t->mode0);
    *mipmap = (GXBool)((t->flags & 1) == 1);
}
void GXGetTexObjLODAll(const GXTexObj* tex_obj, GXTexFilter* min_filt, GXTexFilter* mag_filt, f32* min_lod, f32* max_lod, f32* lod_bias,
                       GXBool* bias_clamp, GXBool* do_edge_lod, GXAnisotropy* max_aniso) {
    s16 tmp;
    if(!tex_obj)throw std::invalid_argument("Null texture object"); NativeTextureObject local;std::memcpy(&local,tex_obj,sizeof(local));auto* t=&local;

    *min_filt = (GXTexFilter)HW2GXFiltConv[TX_SETMODE0_GET_MIN_FILTER(t->mode0)];
    *mag_filt = (GXTexFilter)TX_SETMODE0_GET_MAG_FILTER(t->mode0);
    *min_lod = TX_SETMODE1_GET_MINLOD(t->mode1) / 16.0f;
    *max_lod = TX_SETMODE1_GET_MAXLOD(t->mode1) / 16.0f;
    tmp = (s16)TX_SETMODE0_GET_LODBIAS(t->mode0);
    *lod_bias = ((f32)((s8)tmp) / 32.0f);
    *bias_clamp = (GXBool)TX_SETMODE0_GET_LODCLAMP(t->mode0);
    *do_edge_lod = (GXBool)!TX_SETMODE0_GET_DIAGLOD_ENABLE(t->mode0);
    *max_aniso = (GXAnisotropy)TX_SETMODE0_GET_MAXANISO(t->mode0);
}
u32 GXGetTexObjTlut(const GXTexObj* tex_obj) {
    if(!tex_obj)throw std::invalid_argument("Null texture object"); NativeTextureObject local;std::memcpy(&local,tex_obj,sizeof(local));auto* t=&local;
    return t->tlutName;
}

extern "C" void GXInitTexObjCI(GXTexObj* object,void* image,u16 width,u16 height,GXCITexFmt format,GXTexWrapMode wrapS,GXTexWrapMode wrapT,GXBool mipmap,u32 palette){
 if(!object || (unsigned(format)!=8 && unsigned(format)!=9 && unsigned(format)!=10))throw std::invalid_argument("Invalid indexed texture object");
 GXTexObj next;GXInitTexObj(&next,image,width,height,GXTexFmt(format),wrapS,wrapT,mipmap);
 NativeTextureObject value;std::memcpy(&value,&next,sizeof(value));value.flags&=~2;value.tlutName=palette;std::memcpy(object,&value,sizeof(value));
}
extern "C" void GXInitTlutObj(GXTlutObj* object,void* palette,GXTlutFmt format,u16 entries){
 if(!object || !palette || unsigned(format)>2 || !entries || entries>16384)throw std::invalid_argument("Invalid palette object");
 auto address=Native::gpuResources().address(palette);Native::gpuResources().resolve(address,size_t(entries)*2);
 GXTlutObjInt value{};SC_TX_SETTLUT_SET_FORMAT(value.tlut,format);SC_TX_LOADTLUT0_SET_TLUT_BASE(value.loadTlut0,address>>5);SC_TX_LOADTLUT0_SET_RID(value.loadTlut0,0x64);value.numEntries=entries;
 static_assert(sizeof(value)==sizeof(GXTlutObj));std::memcpy(object,&value,sizeof(value));
}
#include <array>
static std::array<u32,20> nativePaletteWords{};
static std::array<bool,20> nativePaletteLoaded{};
extern "C" void GXLoadTlut(const GXTlutObj* object,u32 name){
 if(!object || name>=20)throw std::invalid_argument("Invalid palette slot");GXTlutObjInt value;std::memcpy(&value,object,sizeof(value));
 unsigned count=name<16?16:64,offset=name<16?512+16*name:768+64*(name-16);
 if(!value.numEntries || value.numEntries>count*16)throw std::invalid_argument("Palette exceeds default region");
 Native::gpuResources().resolve((value.loadTlut0&0xffffff)<<5,size_t(count)*32);
 u32 next=value.tlut;SC_TX_SETTLUT_SET_TMEM_OFFSET(next,offset);
 GDOverflowCheck(15);GDWriteBPCmd(0xfeffffff);GDWriteBPCmd(value.loadTlut0);GDWriteBPCmd(0x65000000|(count<<10)|offset);
 nativePaletteWords[name]=next;nativePaletteLoaded[name]=true;
}
u32 nativePaletteBinding(u32 name){if(name>=20 || !nativePaletteLoaded[name])throw std::invalid_argument("Palette is not loaded for current GPU resources");return nativePaletteWords[name];}

