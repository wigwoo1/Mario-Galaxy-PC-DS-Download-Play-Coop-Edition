$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content (Join-Path $p 'Petari\src\JSystem\JAudio2\JASHeapCtrl.cpp') -Raw
$source=$source.Replace('(u8*)OSRoundUp32B(pBase)','reinterpret_cast<u8*>((reinterpret_cast<std::uintptr_t>(pBase)+31)&~std::uintptr_t(31))').Replace('(u32(mBase) - u32(pBase))','static_cast<u32>(mBase-static_cast<u8*>(pBase))').Replace('u32(it->mBase) - u32(pCurrent)','static_cast<u32>(it->mBase-pCurrent)')
# Wii global delete[] dispatches to JKRHeap::free. Native CRT delete[] does not
# own these chunks; preserve original matching-heap dispatch at the call site.
if([regex]::Matches($source,[regex]::Escape('delete[] pChunk;')).Count-ne 1){throw 'Generic-pool destructor free anchor changed'}
$source=$source.Replace('delete[] pChunk;','JKRHeap::free(pChunk, nullptr);')
$oldDestructor=@'
JASGenericMemPool::~JASGenericMemPool() {
    void* pChunk = _0;
    while (pChunk != nullptr) {
        void* pNextChunk = *(void**)pChunk;
        JKRHeap::free(pChunk, nullptr);
        pChunk = pNextChunk;
    }
}
'@
$source=$source.Replace("`r`n","`n")
$oldDestructor=$oldDestructor.Replace("`r`n","`n")
if(-not $source.Contains($oldDestructor)){throw 'Generic-pool lifetime destructor anchor changed'}
$source=$source.Replace($oldDestructor,$oldDestructor.Replace("    }`n}","    }`n    Native::releaseAudioPoolHeap(this);`n}"))
$allocationAnchor="void JASGenericMemPool::newMemPool(u32 size, int n) {`n"
if([regex]::Matches($source,[regex]::Escape($allocationAnchor)).Count-ne 1){throw 'Generic-pool lifetime allocation anchor changed'}
$source=$source.Replace($allocationAnchor,$allocationAnchor+"    if (n > 0) Native::retainAudioPoolHeap(this, JASDram);`n")
Set-Content (Join-Path $n 'src\generated\AudioHeapKernel.cpp') ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#include <Native/AudioPoolHeapLifetime.hpp>'+[Environment]::NewLine+'#include <cstdint>'+[Environment]::NewLine+$source)
