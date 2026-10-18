$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"

& "$n\generate-native-stream-file-table.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -DGALAXY_NATIVE_JAI_SOUND_ID=1 -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\stream_file_table_tests.cpp" "$n\src\generated\JAUStreamFileTable.cpp" "$p\Petari\src\JSystem\JAudio2\JAIStreamDataMgr.cpp" -o "$n\build\stream-file-table-tests.exe" *> "$n\build\stream-file-table-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\stream-file-table-tests.log" -Tail 20;exit 1}
& "$n\build\stream-file-table-tests.exe"
exit $LASTEXITCODE




