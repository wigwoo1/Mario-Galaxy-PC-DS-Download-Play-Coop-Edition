$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-camera-pointer-candidate.ps1"
$out="$n\build\camera-pointer-candidate"
$s=Get-Content "$out\src\CameraParamChunk.cpp" -Raw
$m=[regex]::Match($s,'(?s)CameraGeneralParam& CameraGeneralParam::operator=.*?\n\}')
if(!$m.Success){throw 'Missing original camera assignment body'}
Set-Content "$out\src\CameraAssignment.cpp" ("#include <revolution/types.h>`n#include `"Game/Camera/CameraParamChunk.hpp`"`n"+$m.Value)
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$out\include" "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h "$n\src\camera_pointer_candidate_tests.cpp" "$out\src\CameraAssignment.cpp" "$p\Petari\src\Game\Camera\CameraParamString.cpp" -o "$out\camera-pointer-tests.exe" *> "$out\tests-build.log"
if($LASTEXITCODE){Get-Content "$out\tests-build.log" -Tail 20;exit 1}
& "$out\camera-pointer-tests.exe"
exit $LASTEXITCODE
