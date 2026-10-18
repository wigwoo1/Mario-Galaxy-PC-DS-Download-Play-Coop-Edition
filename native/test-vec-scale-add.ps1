$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"

& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\vec_scale_add_tests.cpp" "$n\src\native_vec_scale_add.cpp" -o "$n\build\vec-scale-add-tests.exe" *> "$n\build\vec-scale-add-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\vec-scale-add-tests.log" -Tail 20;exit 1}
& "$n\build\vec-scale-add-tests.exe"
exit $LASTEXITCODE


