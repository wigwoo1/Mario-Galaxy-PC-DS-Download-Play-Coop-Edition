$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -DGALAXY_NATIVE_JAI_SOUND_ID=1 -include cmath -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\retail_sound_name_tests.cpp"  "$n\src\generated\SoundTableKernel.cpp" "$n\src\generated\SoundNameConverterKernel.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" -o "$n\build\retail-sound-table-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\retail-sound-table-tests.exe" "$n\build\retail-sound.bstn"
exit $LASTEXITCODE






