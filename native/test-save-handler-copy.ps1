$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"

& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\save_handler_copy_tests.cpp" "$p\Petari\src\Game\System\SaveDataFileAccessor.cpp" "$n\src\generated\SaveHandlerCopyTestProvider.cpp" -o "$n\build\save-handler-copy-tests.exe" *> "$n\build\save-handler-copy-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\save-handler-copy-tests.log" -Tail 20;exit 1}
& "$n\build\save-handler-copy-tests.exe"
exit $LASTEXITCODE




