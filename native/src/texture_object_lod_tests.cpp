#include <Native/GxCommandState.hpp>
#include <revolution/gd/GDBase.h>
#include <Native/GpuResources.hpp>
#include <revolution/gx.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <limits>
#include <stdexcept>
int main(){
 unsigned char image[1024]={};auto address=Native::gpuResources().add(image,sizeof(image));
 GXTexObj initialized{};GXInitTexObj(&initialized,image,16,8,GX_TF_RGBA8,GX_REPEAT,GX_MIRROR,GX_FALSE);
 assert((initialized.dummy[0]&15)==9);assert((initialized.dummy[2]&1023)==15 && (initialized.dummy[2]>>10&1023)==7 && (initialized.dummy[2]>>20&15)==6);assert((initialized.dummy[3]&0xffffff)==address/32);
 // Regeneration must preserve the depth formats used by MainLoopFramework.
 for(auto format:{GX_TF_Z8,GX_TF_Z16,GX_TF_Z24X8}){
  GXTexObj depth{};GXInitTexObj(&depth,image,16,8,format,GX_CLAMP,GX_CLAMP,GX_FALSE);
  void* retainedImage=nullptr;u16 w=0,h=0;GXTexFmt got{};GXTexWrapMode wrapS{},wrapT{};GXBool mip{};
  GXGetTexObjAll(&depth,&retainedImage,&w,&h,&got,&wrapS,&wrapT,&mip);
  assert(retainedImage==image&&w==16&&h==8&&got==format&&mip==GX_FALSE);
 }
 unsigned char shortDepth[32]{};Native::gpuResources().add(shortDepth,sizeof(shortDepth));GXTexObj depth{};
 bool depthRejected=false;try{GXInitTexObj(&depth,shortDepth,4,4,GX_TF_Z24X8,GX_CLAMP,GX_CLAMP,GX_FALSE);}catch(const std::exception&){depthRejected=true;}assert(depthRejected);Native::gpuResources().remove(shortDepth);
 puts("PASS regenerated texture objects preserve Z8/Z16/Z24 formats, CPU pointer roundtrip and depth storage bounds");
 auto retained=initialized;bool initRejected=false;try{GXInitTexObj(&initialized,image,1024,1024,GX_TF_RGBA8,GX_CLAMP,GX_CLAMP,GX_FALSE);}catch(const std::exception&){initRejected=true;}assert(initRejected && !std::memcmp(&retained,&initialized,sizeof(initialized)));
 GXInitTexObj(&initialized,image,8,8,GX_TF_I4,GX_CLAMP,GX_CLAMP,GX_TRUE);assert((initialized.dummy[1]>>8&255)==48);assert(initialized.dummy[3]==address/32);unsigned char commands[128];GDLObj list;GDInitGDLObj(&list,commands,sizeof(commands));GDSetCurrent(&list);Native::GxCommandState state;
for(unsigned slot=0;slot<8;++slot){GDSetCurrOffset(0);GXLoadTexObj(&initialized,GXTexMapID(slot));assert(GDGetCurrOffset()==25);state.consume(commands,25);auto loaded=state.texture(slot);auto sampler=state.sampler(slot);assert(sampler.wrapS==0 && sampler.wrapT==0 && sampler.minFilter==5 && sampler.magFilter==1);assert(loaded.width==8 && loaded.height==8 && loaded.format==0 && loaded.address==address);}
for(unsigned filter=0;filter<6;++filter){GXInitTexObjLOD(&initialized,GXTexFilter(filter),GX_NEAR,0,0,0,GX_FALSE,GX_FALSE,GX_ANISO_1);GDSetCurrOffset(0);GXLoadTexObj(&initialized,GX_TEXMAP0);state.consume(commands,25);assert(state.sampler(0).minFilter==filter && state.sampler(0).magFilter==0);}
Native::gpuResources().remove(image);GDSetCurrOffset(0);bool stale=false;try{GXLoadTexObj(&initialized,GX_TEXMAP0);}catch(const std::exception&){stale=true;}assert(stale && GDGetCurrOffset()==0);
puts("PASS native texture loading: all eight slots and stale-resource rejection before emission");
 puts("PASS native texture initialization: dimensions/format/wrap, registered address, mip LOD and bounded extent rejection");
 GXTexObj object;for(unsigned i=0;i<8;++i)object.dummy[i]=0x12345678+i;auto original=object;GXInitTexObjLOD(&object,GX_LINEAR,GX_NEAR,2,4,0,GX_FALSE,GX_FALSE,GX_ANISO_1);assert((object.dummy[0]>>5&7)==4);assert((object.dummy[0]>>4&1)==0);assert((object.dummy[1]&255)==32 && (object.dummy[1]>>8&255)==64);for(unsigned i=2;i<8;++i)assert(object.dummy[i]==original.dummy[i]);
for(unsigned filter=0;filter<6;++filter){unsigned mapping[]={0,4,1,5,2,6};GXInitTexObjLOD(&object,GXTexFilter(filter),GX_LINEAR,-1,20,10,GX_TRUE,GX_TRUE,GX_ANISO_4);assert((object.dummy[0]>>5&7)==mapping[filter]);assert((object.dummy[0]>>4&1)==1);assert((object.dummy[1]&255)==0 && (object.dummy[1]>>8&255)==160);}
auto saved=object;bool rejected=false;try{GXInitTexObjLOD(&object,GXTexFilter(6),GX_NEAR,0,0,0,GX_FALSE,GX_FALSE,GX_ANISO_1);}catch(const std::invalid_argument&){rejected=true;}assert(rejected && !std::memcmp(&saved,&object,sizeof(object)));rejected=false;try{GXInitTexObjLOD(&object,GX_NEAR,GX_NEAR,0,0,std::numeric_limits<float>::quiet_NaN(),GX_FALSE,GX_FALSE,GX_ANISO_1);}catch(const std::invalid_argument&){rejected=true;}assert(rejected && !std::memcmp(&saved,&object,sizeof(object)));puts("PASS native SDK texture-object LOD: filters, clamp, preserved image fields and invalid-input atomicity");}




