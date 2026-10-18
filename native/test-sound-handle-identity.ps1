$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-animation-sound-object.ps1"
$s=Get-Content "$n\src\generated\SoundHandlesKernel.cpp" -Raw
$start=$s.IndexOf('JAISoundHandle* JAISoundHandles::getHandleUserData(');$open=$s.IndexOf('{',$start);$depth=1;$end=$open+1
while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
$fixture=@'
#include <cstdint>
#include <cassert>
#include <cstdio>
// Lookup algorithm fixture; sound allocation/playback is outside this check.
struct Sound {std::uintptr_t identity;std::uintptr_t getUserData()const{return identity;}};
struct JAISoundHandle {Sound* sound;bool isSoundAttached()const{return sound!=nullptr;}Sound* operator->()const{return sound;}};
struct JAISoundHandles {JAISoundHandle* mHandles;int mNumHandles;JAISoundHandle* getHandleUserData(std::uintptr_t);};
'@
$test=@'
int main(){Sound first{0x100000001ull},second{0x200000001ull};JAISoundHandle handles[]={{nullptr},{&first},{&second}};JAISoundHandles table{handles,3};assert(table.getHandleUserData(first.identity)==&handles[1]);assert(table.getHandleUserData(second.identity)==&handles[2]);assert(table.getHandleUserData(1)==nullptr);handles[1].sound=nullptr;assert(table.getHandleUserData(first.identity)==nullptr);table.mNumHandles=0;assert(table.getHandleUserData(second.identity)==nullptr);std::puts("PASS native sound handle identity lookup: full-width identity, detached, missing and empty table (sound fixture)");}
'@
Set-Content "$n\src\generated\SoundHandleIdentityTests.cpp" ($fixture+[Environment]::NewLine+$s.Substring($start,$end-$start)+[Environment]::NewLine+$test)
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "$n\src\generated\SoundHandleIdentityTests.cpp" -o "$n\build\sound-handle-identity-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\sound-handle-identity-tests.exe"
exit $LASTEXITCODE
