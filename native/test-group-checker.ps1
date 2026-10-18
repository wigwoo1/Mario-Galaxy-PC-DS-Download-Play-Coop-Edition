$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include revolution/os.h "$n\src\group_checker_tests.cpp" "$n\src\generated\CameraNameObjKernel.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$p\Petari\src\Game\Player\GroupChecker.cpp" "$n\src\hash_sort_table_port.cpp" "$n\src\sensor_hash_port.cpp" -o "$n\build\group-checker-tests.exe" *> "$n\build\group-checker-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\group-checker-tests.log" -Tail 20;exit 1}
& "$n\build\group-checker-tests.exe"
exit $LASTEXITCODE
