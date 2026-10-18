$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\native_kpad_input.cpp" "$n\src\native_keyboard_kpad.cpp" "$n\src\native_wpad_port.cpp" "$n\src\vi_port.cpp" -luser32 "$n\src\kpad_input_tests.cpp" -o "$n\build\kpad-input-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\kpad-input-tests.exe"
exit $LASTEXITCODE


