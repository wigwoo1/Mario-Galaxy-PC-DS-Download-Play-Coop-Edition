$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$out = "$n\build\thp-video-tests.exe"
$env:ZIG_GLOBAL_CACHE_DIR = "$n\build\thp-video-cache"
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -Wno-nullability-completeness -Wno-nonportable-include-path -Wno-gcc-compat -Wno-deprecated-declarations -target x86_64-windows-gnu `
        "-I$n\src" "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h `
        "$n\src\native_dvd_port.cpp" "$n\src\thp_video_port.cpp" "$n\src\thp_video_tests.cpp" `
        -lbcrypt -lole32 -lwindowscodecs -o $out *> "$n\build\thp-video-tests.log"
    $compileExit = $LASTEXITCODE
} finally { $ErrorActionPreference = $previousErrorActionPreference }
if ($compileExit) { Get-Content "$n\build\thp-video-tests.log" -Tail 30; exit $compileExit }
& $out 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE
