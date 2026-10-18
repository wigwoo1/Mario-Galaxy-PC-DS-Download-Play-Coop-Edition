#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <Native/TextureCopyState.hpp>
#include <Native/GxCommandState.hpp>
#undef gx
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
void emitNativeTextureCopy(void* dest, GXBool clear,const Native::GxCommandState& registers,Native::GpuResources& resources) {
    struct State:Native::TextureCopyState {u32 zmode,cmode0,peCtrl;};
    State local{};static_cast<Native::TextureCopyState&>(local)=Native::textureCopyState();local.zmode=0x40000000|registers.bp(0x40);local.cmode0=0x41000000|registers.bp(0x41);local.peCtrl=0x43000000|registers.bp(0x43);auto* gx=&local;
    if(unsigned(clear)>1)throw std::invalid_argument("Texture copy clear flag");
    const auto nativeDestination=resources.address(dest);
    const bool changePixel=(gx->cpTexZ && (gx->peCtrl&7)!=3) || ((clear || gx->cpTexZ || (gx->peCtrl&7)==3) && (gx->peCtrl&64));
    GDOverflowCheck(25+(clear?20:0)+(changePixel?10:0));
    u32 reg;
    u32 tempPeCtrl;
    u32 phyAddr;
    GXBool changePeCtrl;

    

    if (clear) {
        reg = gx->zmode;
        SC_PE_ZMODE_SET_ENABLE(reg, 1);
        SC_PE_ZMODE_SET_FUNC(reg, 7);
        GDWriteBPCmd(reg);

        reg = gx->cmode0;
        SC_PE_CMODE0_SET_BLEND_ENABLE(reg, 0);
        SC_PE_CMODE0_SET_LOGICOP_ENABLE(reg, 0);
        GDWriteBPCmd(reg);
    }

    changePeCtrl = false;
    tempPeCtrl = gx->peCtrl;

    if (gx->cpTexZ && (PE_CONTROL_GET_PIXTYPE(tempPeCtrl) != 3)) {
        changePeCtrl = true;
        SC_PE_CONTROL_SET_PIXTYPE(tempPeCtrl, 3);
    }

    if (clear || (PE_CONTROL_GET_PIXTYPE(tempPeCtrl) == 3)) {
        if (PE_CONTROL_GET_ZTOP(tempPeCtrl) == 1) {
            changePeCtrl = true;
            SC_PE_CONTROL_SET_ZTOP(tempPeCtrl, 0);
        }
    }

    if (changePeCtrl) {
        GDWriteBPCmd(tempPeCtrl);
    }

    GDWriteBPCmd(gx->cpTexSrc);
    GDWriteBPCmd(gx->cpTexSize);
    GDWriteBPCmd(gx->cpTexStride);

    phyAddr = nativeDestination;
    reg = 0;
    SC_PE_COPY_DST_BASE_SET_BASE(reg, (phyAddr >> 5));
    SC_PE_COPY_DST_BASE_SET_RID(reg, 0x4B);
    GDWriteBPCmd(reg);

    SC_PE_COPY_CMD_SET_CLEAR(gx->cpTex, clear);
    SC_PE_COPY_CMD_SET_OPCODE(gx->cpTex, 0);
    SC_PE_COPY_CMD_SET_RID(gx->cpTex, 0x52);
    GDWriteBPCmd(gx->cpTex);

    if (clear) {
        GDWriteBPCmd(gx->zmode);
        GDWriteBPCmd(gx->cmode0);
    }

    if (changePeCtrl) {
        GDWriteBPCmd(gx->peCtrl);
    }

    
}

