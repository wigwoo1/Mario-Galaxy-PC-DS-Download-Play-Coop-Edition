$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu -include revolution/types.h "-I$n\include-runtime" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\os_alarm_port.cpp" "$n\src\os_time_port.cpp" "$n\src\os_alarm_tests.cpp" -o "$n\build\os-alarm-tests.exe" *> "$n\build\os-alarm-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\os-alarm-tests.log" -Tail 15;exit 1}
& "$n\build\os-alarm-tests.exe"
exit $LASTEXITCODE
