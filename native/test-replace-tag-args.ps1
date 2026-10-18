param([string]$ProcessorSource)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
if(!$ProcessorSource){$ProcessorSource=Join-Path $p 'Petari\src\Game\Screen\ReplaceTagProcessor.cpp'}
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $n 'build\replace-tag-args-cache'
# Link the same BE message-tag decoder used by the native runtime.
$fixture=Join-Path $n 'src\generated\MessageTagAccessKernel.cpp'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\replace_tag_args_tests.cpp" $ProcessorSource $fixture -lbcrypt -o "$n\build\replace-tag-args-tests.exe" *> "$n\build\replace-tag-args-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\replace-tag-args-tests.log" -Tail 25;exit 1}
& "$n\build\replace-tag-args-tests.exe" "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs"
exit $LASTEXITCODE
