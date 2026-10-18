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

