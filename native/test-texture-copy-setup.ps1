$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-texture-copy-setup.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\texture_copy_setup_tests.cpp" "$n\src\generated\TextureCopySetupKernel.cpp" -o "$n\build\texture-copy-setup-tests.exe" *> "$n\build\texture-copy-setup-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\texture-copy-setup-tests.log" -Tail 20;exit 1}
& "$n\build\texture-copy-setup-tests.exe"
exit $LASTEXITCODE



