& "$PSScriptRoot\generate-texture-pattern-sampler.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\texture_pattern_animation_tests.cpp" "$n\src\generated\TexturePatternSampler.cpp" "$n\src\generated\TexturePatternMaterialMatch.cpp" "$p\Petari\src\JSystem\JUtility\JUTNameTab.cpp" -lbcrypt -o "$n\build\texture-pattern-animation-tests.exe" *> "$n\build\texture-pattern-animation-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\texture-pattern-animation-tests.log" -Tail 20;exit 1}
& "$n\build\texture-pattern-animation-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE






