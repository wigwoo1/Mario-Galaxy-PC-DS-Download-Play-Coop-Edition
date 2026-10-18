#include <revolution/gx.h>
#include <revolution/gx/GXTypes.h>
#include <revolution/gx/GXRegs.h>
#include <revolution/gd/GDBase.h>
#include <revolution/mtx.h>
#include <Native/GpuResources.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

#define GX_TMEM_LO 0x00000
#define GX_TMEM_HI 0x80000
#define GX_32k 0x08000
#define GX_8k 0x02000

namespace {
GXData hostGXData{};
GXFifoObj hostFifo{};
GDLObj hostCommandList{};
__GXFifoObj hostFifoState{};
bool hostInitialized=false;
bool whiteTextureRegistered=false;
GXRenderModeObj nativeRenderMode=[] { GXRenderModeObj mode{}; mode.viTVmode=VI_TVMODE_NTSC_INT; mode.fbWidth=640; mode.efbHeight=480; mode.xfbHeight=480; mode.viWidth=640; mode.viHeight=480; mode.xFBmode=VI_XFBMODE_SF; return mode; }();
}
GXData* const __GXData=&hostGXData;
static GXVtxAttrFmtList GXDefaultVATList[] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},          {GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0}, {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0},
    {GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0},      {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0}, {GX_VA_TEX1, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX2, GX_TEX_ST, GX_F32, 0},          {GX_VA_TEX3, GX_TEX_ST, GX_F32, 0}, {GX_VA_TEX4, GX_TEX_ST, GX_F32, 0},
    {GX_VA_TEX5, GX_TEX_ST, GX_F32, 0},          {GX_VA_TEX6, GX_TEX_ST, GX_F32, 0}, {GX_VA_TEX7, GX_TEX_ST, GX_F32, 0},
    {GX_VA_NULL, (GXCompCnt)0, (GXCompType)0, 0}};
static f32 GXDefaultProjData[] = {1.0f, 0.0f, 1.0f, 0.0f, -1.0f, -2.0f, 0.0f};
static u16 DefaultTexData[] __attribute__((aligned(32))) = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                            0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
static u32 GXTexRegionAddrTable[] = {
    GX_TMEM_LO + 0x00000, GX_TMEM_LO + 0x10000, GX_TMEM_LO + 0x20000, GX_TMEM_LO + 0x30000, GX_TMEM_LO + 0x40000, GX_TMEM_LO + 0x50000,
    GX_TMEM_LO + 0x60000, GX_TMEM_LO + 0x70000, GX_TMEM_LO + 0x08000, GX_TMEM_LO + 0x18000, GX_TMEM_LO + 0x28000, GX_TMEM_LO + 0x38000,
    GX_TMEM_LO + 0x48000, GX_TMEM_LO + 0x58000, GX_TMEM_LO + 0x68000, GX_TMEM_LO + 0x78000, GX_TMEM_LO + 0x00000, GX_TMEM_HI + 0x10000,
    GX_TMEM_LO + 0x20000, GX_TMEM_HI + 0x30000, GX_TMEM_LO + 0x40000, GX_TMEM_HI + 0x18000, GX_TMEM_LO + 0x60000, GX_TMEM_HI + 0x38000,
    GX_TMEM_HI + 0x00000, GX_TMEM_LO + 0x10000, GX_TMEM_HI + 0x20000, GX_TMEM_LO + 0x30000, GX_TMEM_HI + 0x08000, GX_TMEM_LO + 0x50000,
    GX_TMEM_HI + 0x28000, GX_TMEM_LO + 0x70000, GX_TMEM_LO + 0x00000, GX_TMEM_HI + 0x10000, GX_TMEM_LO + 0x20000, GX_TMEM_HI + 0x30000,
    GX_TMEM_LO + 0x40000, GX_TMEM_HI + 0x10000, GX_TMEM_LO + 0x60000, GX_TMEM_HI + 0x30000, GX_TMEM_HI + 0x00000, GX_TMEM_LO + 0x10000,
    GX_TMEM_HI + 0x20000, GX_TMEM_LO + 0x30000, GX_TMEM_HI + 0x00000, GX_TMEM_LO + 0x50000, GX_TMEM_HI + 0x20000, GX_TMEM_LO + 0x70000,
};
static GXTexRegion* __GXDefaultTexRegionCallback(const GXTexObj* t_obj, GXTexMapID id) {
    GXTexFmt fmt;
    GXBool mm;
    fmt = GXGetTexObjFmt(t_obj);
    mm = GXGetTexObjMipMap(t_obj);
    id = (GXTexMapID)(id % 8);

    switch (fmt) {
    case GX_TF_RGBA8:
        if (mm) {
            return &gx->TexRegions2[id];
        } else {
            return &gx->TexRegions1[id];
        }
        break;
    case GX_TF_C4:
    case GX_TF_C8:
    case GX_TF_C14X2:
        return &gx->TexRegions0[id];
        break;
    default:
        if (mm) {
            return &gx->TexRegions1[id];
        } else {
            return &gx->TexRegions0[id];
        }
        break;
    }
}
static GXTlutRegion* __GXDefaultTlutRegionCallback(u32 idx) {
    if (idx >= 20) {
        return 0;
    }

    return &gx->TlutRegions[idx];
}
void __GXInitGX() {
    GXRenderModeObj* rmode = &nativeRenderMode;
    GXTexObj tex_obj;
    Mtx identity_mtx;
    GXColor clear = {64, 64, 64, 255};
    GXColor black = {0, 0, 0, 0};
    GXColor white = {255, 255, 255, 255};
    u32 i;

    GXSetCopyClear(clear, 0xFFFFFF);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, 0x7D);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, 0, 0x7D);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, 0, 0x7D);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX3, GX_IDENTITY, 0, 0x7D);
    GXSetTexCoordGen2(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX4, GX_IDENTITY, 0, 0x7D);
    GXSetTexCoordGen2(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX5, GX_IDENTITY, 0, 0x7D);
    GXSetTexCoordGen2(GX_TEXCOORD6, GX_TG_MTX2x4, GX_TG_TEX6, GX_IDENTITY, 0, 0x7D);
    GXSetTexCoordGen2(GX_TEXCOORD7, GX_TG_MTX2x4, GX_TG_TEX7, GX_IDENTITY, 0, 0x7D);
    GXSetNumTexGens(1);
    GXClearVtxDesc();
    GXInvalidateVtxCache();

    for (i = 9; i <= 24; i++) {
        GXSetArray((GXAttr)i, (void*)gx, 0);
    }

    for (i = 0; i < 8; i++) {
        GXSetVtxAttrFmtv((GXVtxFmt)i, GXDefaultVATList);
    }

    GXSetLineWidth(6, GX_TO_ZERO);
    GXSetPointSize(6, GX_TO_ZERO);
    GXEnableTexOffsets(GX_TEXCOORD0, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD1, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD2, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD3, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD4, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD5, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD6, GX_DISABLE, GX_DISABLE);
    GXEnableTexOffsets(GX_TEXCOORD7, GX_DISABLE, GX_DISABLE);

    identity_mtx[0][0] = 1.0f;
    identity_mtx[0][1] = 0.0f;
    identity_mtx[0][2] = 0.0f;
    identity_mtx[0][3] = 0.0f;
    identity_mtx[1][0] = 0.0f;
    identity_mtx[1][1] = 1.0f;
    identity_mtx[1][2] = 0.0f;
    identity_mtx[1][3] = 0.0f;
    identity_mtx[2][0] = 0.0f;
    identity_mtx[2][1] = 0.0f;
    identity_mtx[2][2] = 1.0f;
    identity_mtx[2][3] = 0.0f;

    GXLoadPosMtxImm(identity_mtx, GX_PNMTX0);
    GXLoadNrmMtxImm(identity_mtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXLoadTexMtxImm(identity_mtx, GX_IDENTITY, GX_MTX3x4);
    GXLoadTexMtxImm(identity_mtx, GX_PTIDENTITY, GX_MTX3x4);
    GXSetViewport(0.0f, 0.0f, (f32)rmode->fbWidth, (f32)rmode->xfbHeight, 0.0f, 1.0f);
    GXSetProjectionv(GXDefaultProjData);

    __GXSetGenMode();
    GXSetCoPlanar(GX_DISABLE);
    GXSetCullMode(GX_CULL_BACK);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GXSetScissorBoxOffset(0, 0);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, black);
    GXSetChanMatColor(GX_COLOR0A0, white);
    GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR1A1, black);
    GXSetChanMatColor(GX_COLOR1A1, white);
    GXInvalidateTexAll();

    GXSetTexRegionCallback(__GXDefaultTexRegionCallback);
    GXSetTlutRegionCallback(__GXDefaultTlutRegionCallback);

    GXInitTexObj(&tex_obj, DefaultTexData, 4, 4, GX_TF_IA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXLoadTexObj(&tex_obj, GX_TEXMAP0);
    GXLoadTexObj(&tex_obj, GX_TEXMAP1);
    GXLoadTexObj(&tex_obj, GX_TEXMAP2);
    GXLoadTexObj(&tex_obj, GX_TEXMAP3);
    GXLoadTexObj(&tex_obj, GX_TEXMAP4);
    GXLoadTexObj(&tex_obj, GX_TEXMAP5);
    GXLoadTexObj(&tex_obj, GX_TEXMAP6);
    GXLoadTexObj(&tex_obj, GX_TEXMAP7);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP3, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD4, GX_TEXMAP4, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD5, GX_TEXMAP5, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD6, GX_TEXMAP6, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE7, GX_TEXCOORD7, GX_TEXMAP7, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE8, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE9, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE10, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE11, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE12, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE13, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE14, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE15, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);

    for (i = 0; i < 16; i++) {
        GXSetTevKColorSel((GXTevStageID)i, GX_TEV_KCSEL_1_4);
        GXSetTevKAlphaSel((GXTevStageID)i, GX_TEV_KASEL_1);
        GXSetTevSwapMode((GXTevStageID)i, GX_TEV_SWAP0, GX_TEV_SWAP0);
    }

    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);

    for (i = 0; i < 16; i++) {
        GXSetTevDirect((GXTevStageID)i);
    }

    GXSetNumIndStages(0);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE2, GX_ITS_1, GX_ITS_1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE3, GX_ITS_1, GX_ITS_1);

    GXSetFog(GX_FOG_NONE, 0.0f, 1.0f, 0.1f, 1.0f, black);
    GXSetFogRangeAdj(GX_DISABLE, 0, 0);
    GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetColorUpdate(GX_ENABLE);
    GXSetAlphaUpdate(GX_ENABLE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_TRUE);
    GXSetDither(GX_ENABLE);
    GXSetDstAlpha(GX_DISABLE, 0);
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GXSetFieldMask(GX_ENABLE, GX_ENABLE);
    GXSetFieldMode((GXBool)(rmode->field_rendering), (GXBool)(((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE)));
    GXSetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GXSetDispCopyDst(rmode->fbWidth, rmode->efbHeight);
    GXSetDispCopyYScale((f32)(rmode->xfbHeight) / (f32)(rmode->efbHeight));
    GXSetCopyClamp((GXFBClamp)(GX_CLAMP_TOP | GX_CLAMP_BOTTOM));
    GXSetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GXSetDispCopyGamma(GX_GM_1_0);
    GXSetDispCopyFrame2Field(GX_COPY_PROGRESSIVE);
    GXClearBoundingBox();

    GXPokeColorUpdate(GX_TRUE);
    GXPokeAlphaUpdate(GX_TRUE);
    GXPokeDither(GX_FALSE);
    GXPokeBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ONE, GX_LO_SET);
    GXPokeAlphaMode(GX_ALWAYS, 0);
    GXPokeAlphaRead(GX_READ_FF);
    GXPokeDstAlpha(GX_DISABLE, 0);
    GXPokeZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);

    GXSetGPMetric(GX_PERF0_NONE, GX_PERF1_NONE);
    GXClearGPMetric();
}
extern "C" GXFifoObj* GXInit(void* base,u32 size){
    if(!base||size<4096||(reinterpret_cast<std::uintptr_t>(base)&31u)||reinterpret_cast<std::uintptr_t>(base)>std::numeric_limits<std::uintptr_t>::max()-size)return nullptr;
    auto* previous=__GDCurrentDL;
    std::memset(&hostGXData,0,sizeof(hostGXData));
    hostGXData.inDispList=GX_FALSE;hostGXData.dlSaveContext=GX_TRUE;hostGXData.abtWaitPECopy=GX_TRUE;
    hostGXData.tcsManEnab=0;hostGXData.tevTcEnab=0;
    GXSetMisc(GX_MT_XF_FLUSH,GX_XF_FLUSH_NONE);
    u32 i=0;    gx->genMode = 0;
    SC_GEN_MODE_SET_RID(gx->genMode, 0);
    gx->bpMask = 255;
    SC_BP_IMASK_SET_RID(gx->bpMask, 0xF);
    gx->lpSize = 0;
    SC_SU_LPSIZE_SET_RID(gx->lpSize, 0x22);

    for (i = 0; i < 16; i++) {
        gx->tevc[i] = 0;
        gx->teva[i] = 0;
        gx->tref[i / 2] = 0;
        gx->texmapId[i] = GX_TEXMAP_NULL;
        SC_TEV_COLOR_ENV_SET_RID(gx->tevc[i], (0xC0 + i * 2));
        SC_TEV_ALPHA_ENV_SET_RID(gx->teva[i], (0xC1 + i * 2));
        SC_TEV_KSEL_SET_RID(gx->tevKsel[i / 2], (0xF6 + i / 2));
        SC_RAS1_TREF_SET_RID(gx->tref[i / 2], (0x28 + i / 2));
    }

    gx->iref = 0;
    SC_RAS1_IREF_SET_RID(gx->iref, 0x27);

    for (i = 0; i < 8; i++) {
        gx->suTs0[i] = 0;
        gx->suTs1[i] = 0;
        SC_SU_TS0_SET_RID(gx->suTs0[i], 0x30 + i * 2);
        SC_SU_TS1_SET_RID(gx->suTs1[i], 0x31 + i * 2);
    }

    SC_SU_SCIS0_SET_RID(gx->suScis0, 0x20);
    SC_SU_SCIS1_SET_RID(gx->suScis1, 0x21);
    SC_PE_CMODE0_SET_RID(gx->cmode0, 0x41);
    SC_PE_CMODE1_SET_RID(gx->cmode1, 0x42);
    SC_PE_ZMODE_SET_RID(gx->zmode, 0x40);
    SC_PE_CONTROL_SET_RID(gx->peCtrl, 0x43);
    SC_PE_COPY_CMD_SET_GAMMA(gx->cpTex, 0);

    gx->zScale = (f32)(0xFFFFFF + 1);
    gx->zOffset = 0.0f;
    gx->dirtyState = 0;
    gx->dirtyVAT = 0;


    for(i=0;i<8;++i){
        GXInitTexCacheRegion(&hostGXData.TexRegions0[i],GX_FALSE,GXTexRegionAddrTable[i],GX_TEXCACHE_32K,GXTexRegionAddrTable[i+8],GX_TEXCACHE_32K);
        GXInitTexCacheRegion(&hostGXData.TexRegions1[i],GX_FALSE,GXTexRegionAddrTable[i+16],GX_TEXCACHE_32K,GXTexRegionAddrTable[i+24],GX_TEXCACHE_32K);
        GXInitTexCacheRegion(&hostGXData.TexRegions2[i],GX_TRUE,GXTexRegionAddrTable[i+32],GX_TEXCACHE_32K,GXTexRegionAddrTable[i+40],GX_TEXCACHE_32K);
    }
    for(i=0;i<16;++i)GXInitTlutRegion(&hostGXData.TlutRegions[i],0x80000+0x8000*8+0x2000*i,GX_TLUT_256);
    for(i=0;i<4;++i)GXInitTlutRegion(&hostGXData.TlutRegions[i+16],0x80000+0x8000*8+0x2000*16+0x8000*i,GX_TLUT_1K);
    try {
        if(!whiteTextureRegistered){Native::gpuResources().addNativeWords(DefaultTexData,sizeof(DefaultTexData));whiteTextureRegistered=true;}
        GDInitGDLObj(&hostCommandList,static_cast<u8*>(base),size);
        GDSetCurrent(&hostCommandList);
        hostFifoState={};hostFifoState.base=static_cast<u8*>(base);hostFifoState.top=static_cast<u8*>(base)+size;hostFifoState.size=size;
        hostFifoState.hiWatermark=size-16*1024;hostFifoState.loWatermark=size/2&~31u;
        hostFifoState.rdPtr=base;hostFifoState.wrPtr=base;hostFifoState.bind_cpu=GX_TRUE;hostFifoState.bind_gp=GX_TRUE;
        static_assert(sizeof(hostFifoState)<=sizeof(hostFifo),"Native FIFO state must fit the SDK opaque FIFO object");
        std::memcpy(&hostFifo,&hostFifoState,sizeof(hostFifoState));
        __GXInitGX();
        hostInitialized=true;
        return &hostFifo;
    } catch(...) {
        GDSetCurrent(previous);
        hostInitialized=false;
        return nullptr;
    }
}extern "C" GXBool GXGetCPUFifo(GXFifoObj* output){
    if(!hostInitialized||!output)return GX_FALSE;
    std::memcpy(output,&hostFifo,sizeof(hostFifo));return GX_TRUE;
}
extern "C" GXBool GXGetGPFifo(GXFifoObj* output){return GXGetCPUFifo(output);}
extern "C" void GXGetFifoPtrs(const GXFifoObj* fifo,void** readPtr,void** writePtr){
    if(!fifo||!readPtr||!writePtr)return;
    __GXFifoObj state{};std::memcpy(&state,fifo,sizeof(state));
    *readPtr=state.rdPtr;*writePtr=hostInitialized&&__GDCurrentDL==&hostCommandList?__GDCurrentDL->ptr:state.wrPtr;
}
extern "C" void GXSetMisc(GXMiscToken token,u32 value){
    switch(token){
    case GX_MT_NULL:break;
    case GX_MT_XF_FLUSH:hostGXData.vNum=value;hostGXData.vNumNot=!hostGXData.vNum;hostGXData.bpSentNot=GX_TRUE;if(value)hostGXData.dirtyState|=8;break;
    case GX_MT_DL_SAVE_CONTEXT:hostGXData.dlSaveContext=value!=0;break;
    case GX_MT_ABORT_WAIT_COPYOUT:hostGXData.abtWaitPECopy=value!=0;break;
    }
}
