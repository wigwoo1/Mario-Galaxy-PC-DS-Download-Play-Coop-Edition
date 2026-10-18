$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-native-content-header.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\content_header_tests.cpp" "$n\src\generated\BinaryDataContentAccessor.cpp" "$n\src\generated\StringKernel.cpp" "$n\src\sensor_hash_port.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" "$n\src\generated\NativeMemoryStreams.cpp" "$p\Petari\src\JSystem\JSupport\JSUOutputStream.cpp" "$p\Petari\src\JSystem\JSupport\JSUInputStream.cpp" "$n\src\jsu_stream_lifecycle_port.cpp" -o "$n\build\content-header-tests.exe" *> "$n\build\content-header-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\content-header-tests.log" -Tail 15;exit 1}
& "$n\build\content-header-tests.exe"
exit $LASTEXITCODE







