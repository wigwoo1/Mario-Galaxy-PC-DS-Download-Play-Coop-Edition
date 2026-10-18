#include <revolution/gx/GXTexture.h>
#include <revolution/gx/GXTypes.h>
#include <private/ra_gen.h>
#include <private/tx_reg.h>
#include <revolution/gx/shortcut_tx_reg.h>
#include <cstring>
#include <stdexcept>
#include <revolution/gd/GDBase.h>
#undef FAST_GPFLAGSET
#define FAST_GPFLAGSET(regOrg,newFlag,regName) ((regOrg)=((regOrg)&~regName##_MASK)|((u32(newFlag)<<regName##_SHIFT)&regName##_MASK))
extern "C" void GXInvalidateTexAll(){
 // Original GXTexture.c invalidates both 256-entry halves of TMEM.
 // Native texture decoding reads the current resource bytes for every draw;
 // no emulated texture-tag cache is retained. Emit the original invalidations
 // with full BP masks, matching the immediate native command-state model.
 GDOverflowCheck(20);
 GDWriteBPCmd(0xfeffffff);
 GDWriteBPCmd(0x66001000);
 GDWriteBPCmd(0x66001100);
 GDWriteBPCmd(0xfeffffff);
}
extern "C" void GXInitTexCacheRegion(GXTexRegion* region,GXBool mipmap,u32 even,GXTexCacheSize evenSize,u32 odd,GXTexCacheSize oddSize){
 auto width=[](GXTexCacheSize size,bool allowNone)->u32{switch(size){case GX_TEXCACHE_32K:return 3;case GX_TEXCACHE_128K:return 4;case GX_TEXCACHE_512K:return 5;case GX_TEXCACHE_NONE:if(allowNone)return 0;default:throw std::invalid_argument("Texture cache size");}};
 if(!region)throw std::invalid_argument("Null texture cache region");
 u32 e=width(evenSize,false),o=width(oddSize,true);
 GXTexRegionInt state;static_assert(sizeof(state)==sizeof(*region));std::memcpy(&state,region,sizeof(state));
 state.image1=((even>>5)&0x7fff)|(e<<15)|(e<<18);
 state.image2=((odd>>5)&0x7fff)|(o<<15)|(o<<18);
 state.is32bMipmap=u8(mipmap);state.isCached=1;
 std::memcpy(region,&state,sizeof(state));
}
extern "C" void GXInitTlutRegion(GXTlutRegion* region,u32 address,GXTlutSize size){
 if(!region || address<0x80000 || (address&0x1ff))throw std::invalid_argument("Invalid TLUT region address");
 const unsigned count=unsigned(size);
 if(!count || count>1024 || (count&(count-1)) || ((address-0x80000)>>9)>0x3ff)
  throw std::invalid_argument("Invalid TLUT region size or TMEM offset");
 GXTlutRegionInt state{};
 SC_TX_LOADTLUT1_SET_TMEM_OFFSET(state.loadTlut1,(address-0x80000)>>9);
 SC_TX_LOADTLUT1_SET_COUNT(state.loadTlut1,count);
 SC_TX_LOADTLUT1_SET_RID(state.loadTlut1,0x65);
 static_assert(sizeof(state)<=sizeof(*region));
 std::memcpy(region,&state,sizeof(state));
}
extern "C" GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback callback){
 GXTexRegionCallback previous=gx->texRegionCallback;gx->texRegionCallback=callback;return previous;
}
extern "C" GXTlutRegionCallback GXSetTlutRegionCallback(GXTlutRegionCallback callback){
 GXTlutRegionCallback previous=gx->tlutRegionCallback;gx->tlutRegionCallback=callback;return previous;
}
