$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-jmath-inline.ps1"
& "$PSScriptRoot\generate-animation-helpers.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp" "$n\src\jmath_inline_tests.cpp"  -o "$n\build\jmath-inline-tests.exe" *> "$n\build\jmath-inline-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\jmath-inline-tests.log" -Tail 20;exit 1}
& "$n\build\jmath-inline-tests.exe"
exit $LASTEXITCODE





