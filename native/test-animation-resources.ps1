$ErrorActionPreference = 'Stop'
$native = $PSScriptRoot
$root = Split-Path $native -Parent
$tmp = Join-Path $env:TEMP 'galaxy-animation-resource-test'
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $tmp 'zig-cache'
New-Item -ItemType Directory -Force -Path $env:ZIG_GLOBAL_CACHE_DIR | Out-Null
$zig = Join-Path $root 'tools\zig-x86_64-windows-0.16.0\zig.exe'
$exe = Join-Path $tmp 'animation-resource-tests.exe'
$log = Join-Path $tmp 'animation-resource-tests.log'
$includes = @(
    "-I$native\include-runtime", "-I$native\include", "-I$root\Petari\include",
    "-I$root\Petari\libs\JSystem\include", "-I$root\Petari\libs\RVL_SDK\include",
    "-I$root\Petari\libs\nw4r\include"
)
$forced = @(
    '-include', 'revolution/types.h',
    '-include', "$native\include-runtime\revolution\os.h",
    '-include', "$native\include-runtime\revolution\gx\GXVert.h",
    '-include', "$native\include-runtime\revolution\os\OSTime.h",
    '-include', "$native\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp",
    '-include', "$native\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp"
)
$sources = @(
    "$native\src\animation_resource_tests.cpp",
    "$native\src\native_bck_animation.cpp",
    "$root\Petari\src\JSystem\JUtility\JUTNameTab.cpp",
    "$root\Petari\src\JSystem\J3DGraphAnimator\J3DAnimation.cpp"
)
& $zig c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections '-Wl,--gc-sections' `
    -target x86_64-windows-gnu @includes @forced @sources -lbcrypt -o $exe *> $log
if ($LASTEXITCODE) {
    Get-Content $log -Tail 60
    exit $LASTEXITCODE
}
& $exe
exit $LASTEXITCODE
