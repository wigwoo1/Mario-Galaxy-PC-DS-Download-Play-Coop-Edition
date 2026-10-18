$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$n\build\zig-cache-os-runtime"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -Wno-nullability-completeness -target x86_64-windows-gnu "-I$n\include-runtime" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_time_port.cpp" "$n\src\os_message_port.cpp" "$n\src\os_thread_port_tests.cpp" -o "$n\build\os-thread-port-tests.exe" *> "$n\build\os-thread-port-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\os-thread-port-tests.log" -Tail 18;exit 1}
& "$n\build\os-thread-port-tests.exe"
exit $LASTEXITCODE



