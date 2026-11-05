$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\RhythmLib\AudMeObject.cpp" -Raw

# Keep the original JKR allocation/free pairing on the native host without
# relying on a compiler-specific array cookie.
$allocation='new(pHeap, 0) AudMeHandle[numHandles]'
if([regex]::Matches($s,[regex]::Escape($allocation)).Count-ne 1 -or [regex]::Matches($s,[regex]::Escape('delete[] mHandles;')).Count-ne 1){throw 'Original AudMeObject handle allocation anchors changed'}
$nativeHandles=@'
namespace {
AudMeHandle* allocateNativeMeHandles(u8 count, JKRHeap* heap) {
    if (!count) return nullptr;
    auto* handles = static_cast<AudMeHandle*>(JKRHeap::alloc(u32(count) * sizeof(AudMeHandle), alignof(AudMeHandle), heap));
    if (!handles) throw std::bad_alloc();
    for (u32 i = 0; i < count; ++i) new (handles + i) AudMeHandle();
    return handles;
}
}
'@
$s=$s.Replace('AudMeObject::AudMeObject(TVec3f*', $nativeHandles+[Environment]::NewLine+'AudMeObject::AudMeObject(TVec3f*')
$s=$s.Replace($allocation,'allocateNativeMeHandles(numHandles, pHeap)')
$s=$s.Replace('delete[] mHandles;','if (mHandles) { for (int i = mNumHandles; i > 0; --i) mHandles[i - 1].~AudMeHandle(); JKRHeap::free(mHandles, nullptr); }')
Set-Content "$n\src\generated\AudMeObject.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)
