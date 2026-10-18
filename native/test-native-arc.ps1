$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu -include revolution/types.h "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\nw4r\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\native_arc_port.cpp" "$n\src\native_arc_tests.cpp" "$n\src\native_jkr_archive_port.cpp" "$n\src\generated\lyt_arcResourceAccessor.cpp" "$p\Petari\src\nw4r\lyt\lyt_resourceAccessor.cpp" "$p\Petari\src\nw4r\ut\ut_LinkList.cpp" -lbcrypt -o "$n\build\native-arc-tests.exe" *> "$n\build\native-arc-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\native-arc-tests.log" -Tail 15;exit 1}
& "$n\build\native-arc-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE





