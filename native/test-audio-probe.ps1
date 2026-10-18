$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\audio_probe_tests.cpp" "$n\src\generated\AudioProbeKernel.cpp" "$n\src\os_critical_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_mutex_port.cpp" -o "$n\build\audio-probe-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\audio-probe-tests.exe"
exit $LASTEXITCODE


