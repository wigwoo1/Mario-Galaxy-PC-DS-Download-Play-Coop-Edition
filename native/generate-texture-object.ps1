$n=$PSScriptRoot;$p=Split-Path $n -Parent
$textureSdk=[IO.File]::ReadAllText("$p\Petari\src\RVL_SDK\gx\GXTexture.c")
$lodStart=$textureSdk.IndexOf('void GXInitTexObjLOD(')
$lodEnd=$textureSdk.IndexOf('// These SDK entry points', $lodStart)
$lodSource=$textureSdk.Substring($lodStart,$lodEnd-$lodStart)
$lodSource=$lodSource.Replace('GX_SETUP_TEXOBJ(t, obj)', 'if(!obj || unsigned(min_filt)>5 || unsigned(mag_filt)>1 || unsigned(max_aniso)>2 || !std::isfinite(min_lod) || !std::isfinite(max_lod) || !std::isfinite(lod_bias))throw std::invalid_argument("Invalid texture LOD arguments"); NativeTextureObject local; std::memcpy(&local,obj,sizeof(local)); auto* t=&local;')
$lodSource=$lodSource.Replace('    SC_TX_SETMODE1_SET_MAXLOD(t->mode1, lmax);','    SC_TX_SETMODE1_SET_MAXLOD(t->mode1, lmax); std::memcpy(obj,&local,sizeof(local));')
$texturePrefix=@"
#include <revolution/gx.h>
#include <revolution/gx/GXRegs.h>
#include <private/ra_gen.h>
#include <revolution/gx/shortcut_tx_reg.h>
#undef FAST_GPFLAGSET
#define FAST_GPFLAGSET(regOrg,newFlag,regName) ((regOrg)=((regOrg)&~regName##_MASK)|((u32(newFlag)<<regName##_SHIFT)&regName##_MASK))
#include <cmath>
#include <cstring>
#include <stdexcept>
struct NativeTextureObject {u32 mode0,mode1,image0,image3,userData,fmt,tlutName;u16 loadCnt;u8 loadFmt,flags;};
static_assert(sizeof(NativeTextureObject)==sizeof(GXTexObj));
static u8 GX2HWFiltConv[]={0,4,1,5,2,6};
"@
[IO.File]::WriteAllText("$n\src\generated\TextureObjectLodKernel.cpp",$texturePrefix+"`n"+$lodSource)

$initStart=$textureSdk.IndexOf('void GXInitTexObj(')
$initEnd=$textureSdk.IndexOf('void GXInitTexObjCI(', $initStart)
$initSource=$textureSdk.Substring($initStart,$initEnd-$initStart)
$initSource=$initSource.Replace('GX_SETUP_TEXOBJ(t, obj)','if(!obj || !image_ptr || !width || !height || width>1024 || height>1024 || unsigned(wrap_s)>2 || unsigned(wrap_t)>2 || unsigned(mipmap)>1)throw std::invalid_argument("Invalid native texture object"); auto nativeAddress=Native::gpuResources().address(image_ptr); Native::gpuResources().resolve(nativeAddress,nativeTextureBytes(width,height,format,mipmap)); NativeTextureObject local{}; auto* t=&local;')
$initSource=$initSource.Replace('memset(t, 0, sizeof(GXTexObjInt));','').Replace('NumLeadingZeros(width)','__builtin_clz(unsigned(width))').Replace('NumLeadingZeros(height)','__builtin_clz(unsigned(height))').Replace('(GX_PHY_ADDR(image_ptr))','nativeAddress').Replace('    t->flags |= 2;','    t->flags |= 2; std::memcpy(obj,&local,sizeof(local));')
$storageStart=$textureSdk.IndexOf('static void __GXGetTexTileShift(')
$storageEnd=$textureSdk.IndexOf('void __GetImageTileCount(', $storageStart)
if($storageStart-lt 0-or $storageEnd-le $storageStart){throw 'Original SDK texture storage helpers missing'}
$textureStorageSource=$textureSdk.Substring($storageStart,$storageEnd-$storageStart).Replace('__GXGetTexTileShift','nativeTextureTileShift').Replace('u32 GXGetTexBufferSize(','static u32 nativeTextureBufferSize(')
$initPrefix=@"
#include <Native/GpuResources.hpp>
$textureStorageSource
#define IsCITexFmt(fmt) (unsigned(fmt)==8 || unsigned(fmt)==9 || unsigned(fmt)==10)
static size_t nativeTextureBytes(unsigned w,unsigned h,GXTexFmt format,bool mipmap){
 switch(format){
 case GX_TF_I4:case GX_TF_I8:case GX_TF_IA4:case GX_TF_IA8:case GX_TF_RGB565:case GX_TF_RGB5A3:case GX_TF_RGBA8:case GX_TF_CMPR:
 case GX_TF_C4:case GX_TF_C8:case GX_TF_C14X2:case GX_TF_Z8:case GX_TF_Z16:case GX_TF_Z24X8:
 case GX_CTF_R4:case GX_CTF_RA4:case GX_CTF_RA8:case GX_CTF_YUVA8:case GX_CTF_A8:case GX_CTF_R8:case GX_CTF_G8:case GX_CTF_B8:case GX_CTF_RG8:case GX_CTF_GB8:
 case GX_CTF_Z4:case GX_CTF_Z8M:case GX_CTF_Z8L:case GX_CTF_Z16L:break;
 default:throw std::invalid_argument("Unsupported native texture format");
 }
 // Share the original SDK's tile-size rules, including Z and copy-texture
 // formats, instead of maintaining a second block-layout table here.
 return nativeTextureBufferSize(u16(w),u16(h),u32(format),mipmap?GX_TRUE:GX_FALSE,mipmap?0xff:1);
}
"@
[IO.File]::WriteAllText("$n\src\generated\TextureObjectLodKernel.cpp",$texturePrefix+"`n"+$initPrefix+"`n"+$initSource+"`n"+$lodSource)

$loadSource=@"
#include <revolution/gd/GDBase.h>
u32 nativePaletteBinding(u32);
extern "C" void GXLoadTexObj(const GXTexObj* object,GXTexMapID slot){
 if(!object || unsigned(slot)>7)throw std::invalid_argument("Invalid texture load slot/object");
 NativeTextureObject value;std::memcpy(&value,object,sizeof(value));
 u32 paletteWord=0;if(!(value.flags&2))paletteWord=nativePaletteBinding(value.tlutName);
 auto width=(value.image0&1023)+1,height=((value.image0>>10)&1023)+1;
 Native::gpuResources().resolve((value.image3&0xffffff)<<5,nativeTextureBytes(width,height,GXTexFmt(value.fmt),bool(value.flags&1)));
 unsigned bank=unsigned(slot)<4?unsigned(slot):unsigned(slot)+28;
 GDOverflowCheck((value.flags&2)?25:30);
 GDWriteBPCmd(0xfeffffff);
 GDWriteBPCmd(((0x80+bank)<<24)|(value.mode0&0xffffff));
 GDWriteBPCmd(((0x84+bank)<<24)|(value.mode1&0xffffff));
 GDWriteBPCmd(((0x88+bank)<<24)|(value.image0&0xffffff));
 GDWriteBPCmd(((0x94+bank)<<24)|(value.image3&0xffffff)); if(!(value.flags&2))GDWriteBPCmd(((0x98+bank)<<24)|(paletteWord&0xffffff));
}
"@
[IO.File]::AppendAllText("$n\src\generated\TextureObjectLodKernel.cpp","`n"+$loadSource)

$extra='static u8 HW2GXFiltConv[]={0,2,4,0,1,3,5,0};'+[Environment]::NewLine
$extra+='extern "C" GXTexFmt GXGetTexObjFmt(const GXTexObj* obj){if(!obj)throw std::invalid_argument("Null texture object");NativeTextureObject local;std::memcpy(&local,obj,sizeof(local));return GXTexFmt(local.fmt);}' + [Environment]::NewLine
$extra+='extern "C" GXBool GXGetTexObjMipMap(const GXTexObj* obj){if(!obj)throw std::invalid_argument("Null texture object");NativeTextureObject local;std::memcpy(&local,obj,sizeof(local));return GXBool((local.flags&1)!=0);}' + [Environment]::NewLine
foreach($name in @('GXInitTexObjTlut','GXGetTexObjAll','GXGetTexObjLODAll','GXGetTexObjTlut')){
$b=$textureSdk.IndexOf($name+'(');$begin=$textureSdk.LastIndexOf([char]10,$b)+1;$o=$textureSdk.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($textureSdk[$e] -eq '{'){$d++};if($textureSdk[$e] -eq '}'){$d--};$e++}
$body=$textureSdk.Substring($begin,$e-$begin)
$body=$body.Replace('GX_SETUP_TEXOBJ(t, obj)','if(!obj)throw std::invalid_argument("Null texture object"); NativeTextureObject local;std::memcpy(&local,obj,sizeof(local));auto* t=&local;').Replace('GX_SETUP_TEXOBJ(t, tex_obj)','if(!tex_obj)throw std::invalid_argument("Null texture object"); NativeTextureObject local;std::memcpy(&local,tex_obj,sizeof(local));auto* t=&local;')
$body=$body.Replace('t->tlutName = tlut_name;','t->tlutName = tlut_name;std::memcpy(obj,&local,sizeof(local));').Replace('*format = t->fmt;','*format = GXTexFmt(t->fmt);').Replace('*image_ptr = (void*)((TX_SETIMAGE3_GET_IMAGE_BASE(t->image3)) << 5);','*image_ptr = const_cast<u8*>(Native::gpuResources().hostAddress((TX_SETIMAGE3_GET_IMAGE_BASE(t->image3)) << 5,1));')
$extra+=$body+[Environment]::NewLine
}
[IO.File]::AppendAllText("$n\src\generated\TextureObjectLodKernel.cpp",[Environment]::NewLine+$extra)
[IO.File]::AppendAllText("$n\src\generated\TextureObjectLodKernel.cpp",[Environment]::NewLine+[IO.File]::ReadAllText("$n\src\palette_object_kernel.inc"))


