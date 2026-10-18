$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\gx_submission_tests.cpp" "$n\src\gx_submission_port.cpp" "$n\src\gd_storage.cpp" "$n\src\gpu_resources.cpp" -o "$n\build\gx-submission-tests.exe" *> "$n\build\gx-submission-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\gx-submission-tests.log" -Tail 20;exit 1}
& "$n\build\gx-submission-tests.exe"
exit $LASTEXITCODE






