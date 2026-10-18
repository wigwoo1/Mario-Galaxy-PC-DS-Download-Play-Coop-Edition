$ErrorActionPreference='Stop';$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-texture-object.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -include revolution/types.h -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\texture_object_lod_tests.cpp" "$n\src\generated\TextureObjectLodKernel.cpp" "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" -o "$n\build\texture-object-lod-tests.exe" *> "$n\build\texture-object-lod-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\texture-object-lod-tests.log" -Tail 12;exit 1}
& "$n\build\texture-object-lod-tests.exe"
exit $LASTEXITCODE



