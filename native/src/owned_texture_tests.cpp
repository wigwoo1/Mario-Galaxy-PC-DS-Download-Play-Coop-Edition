#define main originalHeapTestEntry
#include "unit_heap_live_tests.cpp"
#undef main
#include <JSystem/JUtility/JUTTexture.hpp>
#include <Native/GpuResources.hpp>
#include <Native/TextureCopyBinding.hpp>
#include "preview.hpp"
#include "native_rgb565_copy.hpp"
int main(){
 alignas(32) u8 storage[65536]={};JKRExpHeap heap(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&heap;heap.becomeCurrentHeap();
 const auto initial=heap.getTotalFreeSize();u32 address=0;
 {JUTTexture texture(16,16,GX_TF_RGB565);assert(texture.mTIMG&&texture.mImage&&texture.mFlag==1);assert(texture.getWidth()==16&&texture.getHeight()==16);assert(JKRHeap::findFromRoot(texture._3C)==&heap);address=Native::gpuResources().byteAddress(texture.mImage);assert(Native::gpuResources().resolve(address,512));
 TextureImage source{16,16,6,std::vector<Pixel>(256)};for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++)source.pixels[y*16+x]={u8(x*17),u8(y*17),u8((x+y)*8),255};TextureRenderer renderer(source,true);auto rendered=renderer.readback();
 u8 bytes[64]={};GDLObj commands;GDInitGDLObj(&commands,bytes,sizeof(bytes));GDSetCurrent(&commands);Native::GxCommandState state;
 {Native::TextureCopyBinding binding(state,Native::gpuResources());texture.capture(0,0,GX_TF_RGB565,false,0);}
 assert(GDGetCurrOffset()==30);state.consume(bytes,GDGetCurrOffset());assert(state.pendingCopies().size()==1);assert(state.pendingCopies()[0].destinationAddress()==address);
 writeRgb565Copy(rendered,state.pendingCopies()[0],Native::gpuResources());
 std::vector<u8> encoded(texture.mImage,texture.mImage+512);auto decoded=decodeGxTexture(BinaryView(encoded),16,16,4,0,BinaryView(encoded,0,0));
 for(unsigned i=0;i<256;i++){auto p=rendered.pixels[i];auto expected=rgb565(u16(p.r>>3)<<11|u16(p.g>>2)<<5|p.b>>3);assert(decoded.pixels[i].r==expected.r&&decoded.pixels[i].g==expected.g&&decoded.pixels[i].b==expected.b);}
 assert(std::memcmp(texture.mImage,Native::gpuResources().resolve(address,512),512)==0);GDSetCurrent(nullptr);
 }
 assert(heap.getTotalFreeSize()==initial&&heap.check());bool removed=false;try{Native::gpuResources().resolve(address,1);}catch(const std::out_of_range&){removed=true;}assert(removed);
 const auto generation=Native::gpuResources().generation();bool invalid=false;try{JUTTexture bad(0,16,GX_TF_RGB565);}catch(const std::invalid_argument&){invalid=true;}assert(invalid&&heap.getTotalFreeSize()==initial&&heap.check());assert(Native::gpuResources().generation()==generation+2);
 // Scene heaps release their storage in bulk; JUTTexture is not a JKRDisposer.
 // Its GPU registration must expire even when its C++ destructor is not called.
 auto* scene=JKRSolidHeap::create(16384,&heap,false);assert(scene);scene->becomeCurrentHeap();
 void* firstImage=nullptr;u32 previousAddress=0;
 for(unsigned pass=0;pass<3;++pass){
  auto* texture=new(scene,0) JUTTexture(16,16,GX_TF_RGB565);
  if(!pass)firstImage=texture->mImage;else assert(texture->mImage==firstImage);
  const auto sceneAddress=Native::gpuResources().byteAddress(texture->mImage);assert(sceneAddress>previousAddress);
  scene->freeAll();
  bool expired=false;try{Native::gpuResources().resolve(sceneAddress,1);}catch(const std::out_of_range&){expired=true;}assert(expired);
  assert(scene->mDisposerList.getNumLinks()==0&&scene->check());previousAddress=sceneAddress;
 }
 heap.becomeCurrentHeap();scene->do_destroy();assert(heap.getTotalFreeSize()==initial&&heap.check());
 scene=JKRSolidHeap::create(16384,&heap,false);assert(scene);scene->becomeCurrentHeap();
 auto* texture=new(scene,0) JUTTexture(16,16,GX_TF_RGB565);const auto destroyedAddress=Native::gpuResources().byteAddress(texture->mImage);
 heap.becomeCurrentHeap();scene->do_destroy();
 bool expired=false;try{Native::gpuResources().resolve(destroyedAddress,1);}catch(const std::out_of_range&){expired=true;}assert(expired&&heap.getTotalFreeSize()==initial&&heap.check());
 std::puts("PASS original owned texture constructor/capture/destructor, D3D readback pixels and heap/GPU cleanup");
 std::puts("PASS scene texture GPU cleanup on bulk heap reset, identical-address reuse and heap destruction");
}


