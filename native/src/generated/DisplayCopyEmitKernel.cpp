#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <Native/DisplayCopyState.hpp>
#include <Native/GxCommandState.hpp>
#undef gx
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
void emitNativeDisplayCopy(void* dest, GXBool clear,const Native::GxCommandState& registers,Native::GpuResources& resources) {
    struct State:Native::DisplayCopyState {u32 zmode,cmode0,peCtrl;};
    State local{};static_cast<Native::DisplayCopyState&>(local)=Native::displayCopyState();local.zmode=0x40000000|registers.bp(0x40);local.cmode0=0x41000000|registers.bp(0x41);local.peCtrl=0x43000000|registers.bp(0x43);auto* gx=&local;
    if(unsigned(clear)>1)throw std::invalid_argument("Texture copy clear flag");
    const auto nativeDestination=resources.address(dest);
    const bool changePixel=(clear || (gx->peCtrl&7)==3) && (gx->peCtrl&64);
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
    if ((clear || (PE_CONTROL_GET_PIXTYPE(gx->peCtrl) == 3)) && (PE_CONTROL_GET_ZTOP(gx->peCtrl) == 1)) {
        changePeCtrl = true;
        tempPeCtrl = gx->peCtrl;
        SC_PE_CONTROL_SET_ZTOP(tempPeCtrl, 0);
        GDWriteBPCmd(tempPeCtrl);
    }

    // Set copy parameters.
    GDWriteBPCmd(gx->cpDispSrc);
    GDWriteBPCmd(gx->cpDispSize);
    GDWriteBPCmd(gx->cpDispStride);

    phyAddr = nativeDestination;
    reg = 0;
    SC_PE_COPY_DST_BASE_SET_BASE(reg, (phyAddr >> 5));
    SC_PE_COPY_DST_BASE_SET_RID(reg, 0x4B);
    GDWriteBPCmd(reg);

    SC_PE_COPY_CMD_SET_CLEAR(gx->cpDisp, clear);
    SC_PE_COPY_CMD_SET_OPCODE(gx->cpDisp, 1);
    SC_PE_COPY_CMD_SET_RID(gx->cpDisp, 0x52);
    GDWriteBPCmd(gx->cpDisp);

    if (clear) {
        GDWriteBPCmd(gx->zmode);
        GDWriteBPCmd(gx->cmode0);
    }

    if (changePeCtrl) {
        GDWriteBPCmd(gx->peCtrl);
    }

    
}

