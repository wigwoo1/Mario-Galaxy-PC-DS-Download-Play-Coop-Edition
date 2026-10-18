$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JKernel\JKRDvdAramRipper.hpp" -Raw
$h=$h.Replace('#pragma once','#pragma once'+[Environment]::NewLine+'#include <cstdint>').Replace('void (*mCallback)(u32)','void (*mCallback)(std::uintptr_t)').Replace('void (*)(u32)','void (*)(std::uintptr_t)')
Set-Content "$n\include-runtime\JSystem\JKernel\JKRDvdAramRipper.hpp" $h
$s=Get-Content "$p\Petari\src\JSystem\JKernel\JKRDvdAramRipper.cpp" -Raw
$start=$s.IndexOf('JKRAramBlock* JKRDvdAramRipper::loadToAram')
$end=$s.IndexOf('JKRADCommand* JKRDvdAramRipper::loadToAram_Async',$start)
$s=$s.Remove($start,$end-$start)
$s=$s.Replace('void (*callback)(u32)','void (*callback)(std::uintptr_t)').Replace('reinterpret_cast< u32 >(command)','reinterpret_cast<std::uintptr_t>(command)')
$s=$s.Replace('u8 buffer[0x40];','alignas(32) u8 buffer[0x40];').Replace('reinterpret_cast< u8* >(ALIGN_NEXT(reinterpret_cast< u32 >(&buffer), 0x20))','buffer')
$s=$s.Replace('JKRAramPcs(0, reinterpret_cast< u32 >(dmaBuf), param_1, size, nullptr);','if(!nativeUploadAram(param_1,dmaBuf,size)) throw std::runtime_error("DVD decompression ARAM upload failed");')
$s=$s.Replace('IS_NOT_ALIGNED(size, 0x20)','(size & 0x1f)')
$s=$s.Replace('header->length','((u32(src[4])<<24)|(u32(src[5])<<16)|(u32(src[6])<<8)|src[7])')
foreach($name in @('command','stream','command->mStreamCommand','dvdFile->mFileStream','mDvdFile')){$s=$s.Replace("delete $name;","nativeDvdAramDestroy($name);")}
$s=$s.Replace('new (JKRGetSystemHeap(), -4)','new (JKRGetSystemHeap(), -int(alignof(std::max_align_t)))')
$prefix=@"
#include <Native/HeapPlacement.hpp>
#include <stdexcept>
#include "native_jkr_aram_port.hpp"
template<class T>static void nativeDvdAramDestroy(T* ptr){if(ptr){ptr->~T();JKRHeap::free(ptr,nullptr);}}
"@
Set-Content "$n\src\generated\DvdAramRipperTransferKernel.cpp" ($prefix+[Environment]::NewLine+$s)


