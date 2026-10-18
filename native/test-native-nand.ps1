$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"

& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\native_nand_tests.cpp" "$n\src\native_nand_port.cpp" -o "$n\build\native-nand-tests.exe" *> "$n\build\native-nand-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\native-nand-tests.log" -Tail 20;exit 1}
Push-Location $n
& "$n\build\native-nand-tests.exe"
Pop-Location
exit $LASTEXITCODE



