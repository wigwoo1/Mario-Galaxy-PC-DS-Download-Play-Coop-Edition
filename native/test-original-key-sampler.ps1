$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-jmath-inline.ps1"
& "$PSScriptRoot\generate-animation-sampler.ps1"
& "$PSScriptRoot\generate-animation-helpers.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -DNATIVE_BCK_TEST_NO_BCA -DNATIVE_BCK_TEST_NO_BPK -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp" "$n\src\original_key_sampler_tests.cpp" "$n\src\generated\OriginalTransformSampler.cpp" "$n\src\native_bck_animation.cpp"  -lbcrypt -o "$n\build\original-key-sampler-tests.exe" *> "$n\build\original-key-sampler-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-key-sampler-tests.log" -Tail 20;exit 1}
& "$n\build\original-key-sampler-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE









