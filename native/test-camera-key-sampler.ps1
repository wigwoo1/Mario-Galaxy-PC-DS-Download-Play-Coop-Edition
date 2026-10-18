$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-camera-pointer-candidate.ps1"
$out="$n\build\camera-pointer-candidate"
$s=Get-Content "$out\src\CameraAnim.cpp" -Raw
$start=$s.IndexOf('void CamAnmDataAccessor::set(')
$end=$s.IndexOf('CameraAnim::CameraAnim(')
if($start -lt 0 -or $end -le $start){throw 'Missing original sampler anchors'}
Set-Content "$out\src\KeySampler.cpp" ("#include <cstring>`n#include <Game/Util/MathUtil.hpp>`n#include <Game/Camera/CameraAnim.hpp>`n#include <JSystem/JMath/JMath.hpp>`n"+$s.Substring($start,$end-$start))
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h "$out\src\KeySampler.cpp" "$n\src\camera_key_sampler_tests.cpp" -o "$out\key-sampler-tests.exe" *> "$out\key-sampler-build.log"
if($LASTEXITCODE){Get-Content "$out\key-sampler-build.log" -Tail 20;exit 1}
& "$out\key-sampler-tests.exe"
exit $LASTEXITCODE

