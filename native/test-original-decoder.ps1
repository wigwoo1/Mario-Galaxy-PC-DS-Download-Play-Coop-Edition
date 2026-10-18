$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\original_decoder_tests.cpp" "$n\src\generated\JKRDecoderKernel.cpp" -o "$n\build\original-decoder-tests.exe" *> "$n\build\original-decoder-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-decoder-tests.log" -Tail 20;exit 1}
& "$n\build\original-decoder-tests.exe"
exit $LASTEXITCODE

