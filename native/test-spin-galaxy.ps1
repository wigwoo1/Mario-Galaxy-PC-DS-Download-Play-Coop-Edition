$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-native-spin-storage.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\spin_galaxy_tests.cpp" "$n\src\generated\SpinDriverGalaxyCodecFixture.cpp" "$n\src\generated\StringKernel.cpp"  "$p\Petari\src\JSystem\JGadget\hashcode.cpp" "$n\src\generated\NativeMemoryStreams.cpp" "$p\Petari\src\JSystem\JSupport\JSUOutputStream.cpp" "$p\Petari\src\JSystem\JSupport\JSUInputStream.cpp" "$n\src\jsu_stream_lifecycle_port.cpp" -o "$n\build\spin-galaxy-tests.exe" *> "$n\build\spin-galaxy-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\spin-galaxy-tests.log" -Tail 15;exit 1}
& "$n\build\spin-galaxy-tests.exe"
exit $LASTEXITCODE










