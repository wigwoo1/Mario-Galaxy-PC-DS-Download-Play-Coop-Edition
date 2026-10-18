#include <revolution/gx/GXTypes.h>
#include <revolution/gx/GXLighting.h>
#include <revolution/gd/GDBase.h>
#include <cstring>
#include <stdexcept>
// Native FIFO adaptation of original 16-word XF light load.
extern "C" void GXLoadLightObjImm(const GXLightObj* object,GXLightID id){
 if(!object)throw std::invalid_argument("Null GX light object");
 unsigned bits=unsigned(id),slot=0;while(bits>1){bits>>=1;++slot;}
 if(!unsigned(id)||slot>7)throw std::invalid_argument("Invalid GX light ID");
 GXLightObjInt light;std::memcpy(&light,object,sizeof(light));
 GDOverflowCheck(69);GDWriteXFCmdHdr(u16(0x600+slot*16),16);
 for(unsigned i=0;i<3;++i)GDWrite_u32(0);
 GDWrite_u32(light.Color);
 for(float value:light.a)GDWrite_f32(value);for(float value:light.k)GDWrite_f32(value);
 for(float value:light.lpos)GDWrite_f32(value);for(float value:light.ldir)GDWrite_f32(value);
}



