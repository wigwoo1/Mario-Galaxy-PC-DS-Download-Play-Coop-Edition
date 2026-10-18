$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\original_async_job_tests.cpp" "$n\src\generated\AsyncJobKernel.cpp" "$n\src\os_message_port.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\generated\JKRHeapDispatch.cpp" "$n\src\generated\JKRDisposer.cpp" "$p\Petari\src\JSystem\JSupport\JSUList.cpp" "$n\src\heap_error_port.cpp" "$n\src\os_panic_port.cpp" -o "$n\build\original-async-job-tests.exe" *> "$n\build\original-async-job-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-async-job-tests.log" -Tail 18;exit 1}
& "$n\build\original-async-job-tests.exe"
exit $LASTEXITCODE



