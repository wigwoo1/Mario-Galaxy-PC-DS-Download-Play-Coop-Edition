param([Parameter(Mandatory)][string]$SavePath,[switch]$RequireCreated)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $n 'build\native-save-file-verify-cache'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\native_save_file_verify.cpp" -o "$n\build\native-save-file-verify.exe" *> "$n\build\native-save-file-verify.log"
if($LASTEXITCODE){Get-Content "$n\build\native-save-file-verify.log" -Tail 16;exit 1}
if($RequireCreated){& "$n\build\native-save-file-verify.exe" $SavePath --require-created}else{& "$n\build\native-save-file-verify.exe" $SavePath}
exit $LASTEXITCODE