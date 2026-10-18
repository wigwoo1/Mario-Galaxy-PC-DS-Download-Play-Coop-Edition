$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\AudSoundObjHolder.cpp" -Raw
$body=@('#include <Game/AudioLib/AudSoundObjHolder.hpp>','#include <cassert>','#include <cstdint>','#include <cstdio>')
foreach($sig in @('void AudSoundObjHolder::add(','void AudSoundObjHolder::remove(','void AudSoundObjHolder::moveOver(')){
 $start=$s.IndexOf($sig);if($start -lt 0){throw 'Missing original holder function'};$open=$s.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)
}
$body+=@'
// Storage constructor fixture; actual sound objects are never dereferenced.
AudSoundObjHolder::AudSoundObjHolder(JKRHeap*,s32 capacity):mArray(nullptr),mCapacity(capacity),mSize(0){}
int main(){AudSoundObject* storage[3]{};AudSoundObjHolder holder(nullptr,3);holder.mArray=storage;auto a=reinterpret_cast<AudSoundObject*>(std::uintptr_t(0x100000001ull));auto b=reinterpret_cast<AudSoundObject*>(std::uintptr_t(0x200000001ull));auto c=reinterpret_cast<AudSoundObject*>(std::uintptr_t(0x300000001ull));holder.add(a);holder.add(b);holder.add(c);holder.add(a);assert(holder.mSize==3);holder.remove(b);assert(holder.mSize==2&&storage[0]==a&&storage[1]==c);holder.remove(b);assert(holder.mSize==2);holder.add(a);holder.remove(a);assert(holder.mSize==2&&storage[0]==a&&storage[1]==c);holder.remove(a);holder.remove(c);assert(holder.mSize==0);holder.remove(c);assert(holder.mSize==0);std::puts("PASS original sound holder membership: capacity, full-width identity, compaction, duplicate-last removal and missing entries (storage fixture)");}
'@
Set-Content "$n\src\generated\SoundObjectHolderTests.cpp" ($body -join [Environment]::NewLine)
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$p\Petari\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\generated\SoundObjectHolderTests.cpp" -o "$n\build\sound-object-holder-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\sound-object-holder-tests.exe"
exit $LASTEXITCODE
