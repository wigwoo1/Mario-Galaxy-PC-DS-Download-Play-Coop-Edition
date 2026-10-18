$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\indexed_texture_matrix_tests.cpp" "$n\src\gpu_resources.cpp" "$n\src\gx_indirect_port.cpp" "$n\src\gd_storage.cpp" -ld3dcompiler_47 -ld3d11 -ldxgi -o "$n\build\indexed-texture-matrix-tests.exe" *> "$n\build\indexed-texture-matrix-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\indexed-texture-matrix-tests.log" -Tail 15;exit 1}
& "$n\build\indexed-texture-matrix-tests.exe"
exit $LASTEXITCODE








