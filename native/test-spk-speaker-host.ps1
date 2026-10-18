$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu `
    "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" `
    "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" `
    -include revolution/types.h "$n\src\spk_speaker_ctrl_host_tests.cpp" `
    "$n\src\spk_speaker_ctrl_host.cpp" -o "$n\build\spk-speaker-host-tests.exe" `
    *> "$n\build\spk-speaker-host-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\spk-speaker-host-tests.log" -Tail 20;exit $LASTEXITCODE}
& "$n\build\spk-speaker-host-tests.exe"
exit $LASTEXITCODE
