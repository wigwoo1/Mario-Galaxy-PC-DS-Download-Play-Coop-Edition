$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$p\Petari\libs\nw4r\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\text_stream_tests.cpp" "$p\Petari\src\nw4r\ut\ut_CharStrmReader.cpp" -o "$n\build\text-stream-tests.exe" *> "$n\build\text-stream-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\text-stream-tests.log" -Tail 15;exit 1}
& "$n\build\text-stream-tests.exe"
exit $LASTEXITCODE
