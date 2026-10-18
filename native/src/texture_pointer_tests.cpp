#include <Native/TexturePointers.hpp>
#include <Native/GpuResources.hpp>
#include <JSystem/J3DGraphBase/J3DTexture.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
struct Region {
 u8* bytes;
 explicit Region(uintptr_t address):bytes(static_cast<u8*>(VirtualAlloc(reinterpret_cast<void*>(address),4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE))){if(!bytes)throw std::runtime_error("Could not reserve separated texture test regions");}
 ~Region(){VirtualFree(bytes,0,MEM_RELEASE);}
 ResTIMG* header(){return reinterpret_cast<ResTIMG*>(bytes);}
 void initialize(u8 value){std::memset(bytes,value,4096);ResTIMG h{};h.mFormat=GX_TF_C8;h.mWidth=8;h.mHeight=8;h.mPaletteName=1;h.mPaletteNum=16;h.mPaletteDataOffset=1024;h.mImageDataOffset=32;h.mImageNum=3;h.mMipmap=true;h.mMaxLod=16;std::memcpy(bytes,&h,32);}
};
int main(){
 static_assert(sizeof(ResTIMG)==32);
 Region a(0x10000000000ull),b(0x20000000000ull),c(0x30000000000ull);
 a.initialize(0x11);b.initialize(0x22);c.initialize(0x33);auto& gpu=Native::gpuResources();
 gpu.add(a.bytes,4096);gpu.add(b.bytes,4096);gpu.add(c.bytes,4096);
 J3DTexture first(1,a.header()),second(1,b.header()),third(1,c.header());
 const auto originalA=*a.header(),originalB=*b.header();
 first.setResTIMG(0,*b.header()); // source above destination by1TiB
 assert(Native::textureImage(a.header())==b.bytes+32&&Native::texturePalette(a.header())==b.bytes+1024);
 assert(!std::memcmp(a.header(),&originalB,32));assert(*gpu.resolve(gpu.byteAddress(Native::textureImage(a.header())),1)==0x22);
 third.setResTIMG(0,*a.header()); // chained copy retains the original host image
 assert(Native::textureImage(c.header())==b.bytes+32);assert(Native::texturePalette(c.header())==b.bytes+1024);
 gpu.remove(a.bytes); // removing the intermediate header must not release B
 assert(Native::textureImage(c.header())==b.bytes+32);
 a.initialize(0x11);gpu.add(a.bytes,4096);assert(Native::textureImage(a.header())==a.bytes+32);
 auto bad=originalA;bad.mWidth=4;bool rejected=false;
 try{second.setResTIMG(0,bad);}catch(const std::invalid_argument&){rejected=true;}
 assert(rejected&&!std::memcmp(b.header(),&originalB,32)&&Native::textureImage(b.header())==b.bytes+32);
 second.setResTIMG(0,*a.header()); // source below destination by1TiB
 assert(Native::textureImage(b.header())==a.bytes+32&&Native::texturePalette(b.header())==a.bytes+1024);
 second.setResTIMG(0,*b.header());assert(Native::textureImage(b.header())==a.bytes+32);
 assert(Native::textureImage(c.header())==b.bytes+32); // prior copies are snapshots
 gpu.remove(b.bytes);rejected=false;try{Native::textureImage(c.header());}catch(const std::logic_error&){rejected=true;}assert(rejected);
 third.setResTIMG(0,*a.header());assert(Native::textureImage(c.header())==a.bytes+32);
 gpu.remove(c.bytes);c.initialize(0x44);gpu.add(c.bytes,4096);assert(Native::textureImage(c.header())==c.bytes+32);
 a.header()->mImageDataOffset=0;a.header()->mPaletteNum=0;third.setResTIMG(0,*a.header());
 assert(Native::textureImage(c.header())==a.bytes+32&&Native::texturePalette(c.header())==nullptr);
 assert(c.header()->mImageNum==3&&c.header()->mMipmap&&c.header()->mImageDataOffset==0);
 gpu.remove(a.bytes);rejected=false;try{Native::textureImage(c.header());}catch(const std::logic_error&){rejected=true;}assert(rejected);
 gpu.remove(c.bytes);
 std::puts("PASS actual J3D texture replacement: image/palette pointers1TiB above/below, chains, self-copy, metadata/mips, zero offsets, atomic rejection, owner release and address reuse");
}
