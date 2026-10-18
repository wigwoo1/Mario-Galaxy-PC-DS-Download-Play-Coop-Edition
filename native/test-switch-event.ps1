$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\switch_event_tests.cpp" "$p\Petari\src\Game\Util\SwitchEventFunctorListener.cpp" -o "$n\build\switch-event-tests.exe" *> "$n\build\switch-event-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\switch-event-tests.log" -Tail 20;exit 1}
& "$n\build\switch-event-tests.exe"
exit $LASTEXITCODE

