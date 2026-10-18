#include <revolution/gx.h>
#include <revolution/gx/GXTypes.h>
#include <revolution/gx/GXFrameBuf.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/gx/GXManage.h>
#include <revolution/gd/GDBase.h>
#include <Native/CommandSubmission.hpp>
#include <private/tx_reg.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <stdexcept>
extern "C" void GXWaitDrawDone(void);

static GXData testGXData{};
extern "C" GXData* const __GXData=&testGXData;
static GXTexRegion callbackTextureRegion{};
static GXTlutRegion callbackTlutRegion{};
static GXTexRegion* textureRegionCallback(const GXTexObj*,GXTexMapID){return &callbackTextureRegion;}
static GXTlutRegion* tlutRegionCallback(u32){return &callbackTlutRegion;}
struct CompletionProbe:Native::CommandSubmitter {
    bool acceptedFinish=false,waited=false;
    void submit(const u8*,std::size_t,bool finish)override{acceptedFinish=finish;}
    void waitForFinish()override{assert(acceptedFinish);waited=true;}
};

struct TextureObjectFixture {
    u32 mode0,mode1,image0,image3;
    u32 userData;
    GXTexFmt fmt;
    u32 tlutName;
    u16 loadCnt;
    u8 loadFmt,flags;
};
static_assert(sizeof(TextureObjectFixture)==sizeof(GXTexObj));

int main(){
    TextureObjectFixture encoded{};encoded.fmt=GXTexFmt(9);encoded.flags=1;
    GXTexObj texture{};std::memcpy(&texture,&encoded,sizeof(encoded));
    assert(unsigned(GXGetTexObjFmt(&texture))==9);
    assert(GXGetTexObjMipMap(&texture)==GX_TRUE);
    encoded.flags=0;std::memcpy(&texture,&encoded,sizeof(encoded));
    assert(GXGetTexObjMipMap(&texture)==GX_FALSE);
    bool rejected=false;try{GXGetTexObjFmt(nullptr);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);

    GXTlutRegion region{};
    GXInitTlutRegion(&region,0x88000,GX_TLUT_256);
    GXTlutRegionInt encodedRegion{};std::memcpy(&encodedRegion,&region,sizeof(encodedRegion));
    assert(TX_LOADTLUT1_GET_TMEM_OFFSET(encodedRegion.loadTlut1)==64);
    assert(TX_LOADTLUT1_GET_COUNT(encodedRegion.loadTlut1)==GX_TLUT_256);
    assert(TX_LOADTLUT1_GET_RID(encodedRegion.loadTlut1)==0x65);
    assert(encodedRegion.tlutObj.tlut==0 && encodedRegion.tlutObj.loadTlut0==0 && encodedRegion.tlutObj.numEntries==0);
    auto unchanged=region;rejected=false;
    try{GXInitTlutRegion(&region,0x88001,GX_TLUT_256);}catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && !std::memcmp(&region,&unchanged,sizeof(region)));

    assert(GXSetTexRegionCallback(textureRegionCallback)==nullptr);
    assert(gx->texRegionCallback(&texture,GX_TEXMAP0)==&callbackTextureRegion);
    assert(GXSetTexRegionCallback(nullptr)==textureRegionCallback);
    assert(GXSetTlutRegionCallback(tlutRegionCallback)==nullptr);
    assert(gx->tlutRegionCallback(0)==&callbackTlutRegion);
    assert(GXSetTlutRegionCallback(nullptr)==tlutRegionCallback);

    u8 stateCommands[64]{};GDLObj stateWriter;GDInitGDLObj(&stateWriter,stateCommands,sizeof(stateCommands));GDSetCurrent(&stateWriter);
    GXSetFieldMask(GX_TRUE,GX_FALSE);
    assert(GDGetCurrOffset()==5 && stateCommands[0]==0x61 && stateCommands[1]==0x44 && stateCommands[4]==2);
    gx->genMode=0x12345678;gx->bpSentNot=GX_TRUE;GDSetCurrOffset(0);__GXSetGenMode();
    assert(GDGetCurrOffset()==5 && stateCommands[0]==0x61 && stateCommands[1]==0x12 && stateCommands[4]==0x78 && gx->bpSentNot==GX_FALSE);
    GXSetGPMetric(GX_PERF0_VERTICES,GX_PERF1_VERTICES);
    assert(gx->perf0==GX_PERF0_VERTICES && gx->perf1==GX_PERF1_VERTICES);
    GXClearGPMetric();u32 xfWaitIn=1,xfWaitOut=1,rasBusy=1,clocks=1;
    GXReadXfRasMetric(&xfWaitIn,&xfWaitOut,&rasBusy,&clocks);
    assert(xfWaitIn==0 && xfWaitOut==0 && rasBusy==0 && clocks==0);
    bool badMetricOutput=false;try{GXReadXfRasMetric(nullptr,&xfWaitOut,&rasBusy,&clocks);}catch(const std::invalid_argument&){badMetricOutput=true;}assert(badMetricOutput);
    gx->lpSize=0x22000000;GDSetCurrOffset(0);GXSetFieldMode(GX_TRUE,GX_TRUE);
    assert(GDGetCurrOffset()==10 && gx->lpSize==0x22400000);
    assert(stateCommands[0]==0x61 && stateCommands[1]==0x22 && stateCommands[2]==0x40);
    assert(stateCommands[5]==0x61 && stateCommands[6]==0x68 && stateCommands[9]==1);
    GDSetCurrOffset(0);GXClearBoundingBox();
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0x55 && stateCommands[4]==0xff && stateCommands[6]==0x56 && stateCommands[9]==0xff);
    GDSetCurrOffset(0);GXPokeColorUpdate(GX_TRUE);
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0xfe && stateCommands[6]==0x41 && stateCommands[9]==8);
    GDSetCurrOffset(0);GXPokeAlphaUpdate(GX_TRUE);
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0xfe && stateCommands[6]==0x41 && stateCommands[9]==16);
    GDSetCurrOffset(0);GXPokeDither(GX_TRUE);
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0xfe && stateCommands[6]==0x41 && stateCommands[9]==4);
    GDSetCurrOffset(0);GXPokeBlendMode(GX_BM_BLEND,GX_BL_SRCALPHA,GX_BL_INVSRCALPHA,GX_LO_NOOP);
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0xfe && stateCommands[6]==0x41 && stateCommands[7]==0 && stateCommands[8]==0x54 && stateCommands[9]==0xa1);
    GDSetCurrOffset(0);GXPokeAlphaMode(GX_GREATER,77);
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0xfe && stateCommands[6]==0x43 && stateCommands[8]==4 && stateCommands[9]==77);
    GDSetCurrOffset(0);GXPokeDstAlpha(GX_TRUE,173);
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0xfe && stateCommands[6]==0x42 && stateCommands[8]==1 && stateCommands[9]==173);
    GDSetCurrOffset(0);GXPokeZMode(GX_TRUE,GX_LEQUAL,GX_TRUE);
    assert(GDGetCurrOffset()==10 && stateCommands[1]==0xfe && stateCommands[6]==0x40);
    GXBool overHi=GX_TRUE,underLow=GX_TRUE,readIdle=GX_FALSE,commandIdle=GX_FALSE,breakpoint=GX_TRUE;
    GXGetGPStatus(&overHi,&underLow,&readIdle,&commandIdle,&breakpoint);
    assert(overHi==GX_FALSE && underLow==GX_FALSE && readIdle==GX_TRUE && commandIdle==GX_TRUE && breakpoint==GX_FALSE);
    const auto beforeBreakpointHints=GDGetCurrOffset();
    GXEnableBreakPt(reinterpret_cast<void*>(uintptr_t(0x1234)));
    GXDisableBreakPt();
    assert(GDGetCurrOffset()==beforeBreakpointHints);
    CompletionProbe completion;{
        Native::CommandSubmissionBinding submission(completion);
        GDSetCurrOffset(0);GXSetDrawDone();
        GXGetGPStatus(&overHi,&underLow,&readIdle,&commandIdle,&breakpoint);
        assert(readIdle==GX_FALSE && commandIdle==GX_FALSE);
        GXWaitDrawDone();assert(completion.waited);
        GXGetGPStatus(&overHi,&underLow,&readIdle,&commandIdle,&breakpoint);
        assert(readIdle==GX_TRUE && commandIdle==GX_TRUE);
    }
    GDSetCurrent(nullptr);

    u8 commands[256]{};GDLObj list;GDInitGDLObj(&list,commands,sizeof(commands));GDSetCurrent(&list);
    const GXVtxAttrFmtList formats[]={
        {GX_VA_POS,GX_POS_XYZ,GX_F32,0},
        {GX_VA_TEX0,GX_TEX_ST,GX_U16,5},
        {GX_VA_NULL,GX_POS_XY,GX_U8,0}
    };
    GXSetVtxAttrFmtv(GX_VTXFMT0,formats);
    assert(GDGetCurrOffset()==36);
    assert(commands[0]==8 && commands[1]==0x70);
    assert(commands[6]==8 && commands[7]==0x80);
    assert(commands[12]==8 && commands[13]==0x90);
    assert(commands[18]==8 && commands[19]==0x70);
    assert(commands[24]==8 && commands[25]==0x80);
    assert(commands[30]==8 && commands[31]==0x90);
    GDSetCurrent(nullptr);
    std::puts("PASS host GX default-state helpers: texture metadata, TLUT placement and vector VAT commands");
}
