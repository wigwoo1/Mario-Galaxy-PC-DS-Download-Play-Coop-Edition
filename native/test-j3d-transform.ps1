$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-actor-runtime.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\j3d_transform_tests.cpp" "$n\src\generated\J3DTransformPort.cpp" "$n\src\sdk_matrix_port.cpp" "$p\Petari\src\JSystem\JMath\JMATrigonometric.cpp" -o "$n\build\j3d-transform-tests.exe" *> "$n\build\j3d-transform-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\j3d-transform-tests.log" -Tail 20;exit 1}
& "$n\build\j3d-transform-tests.exe"
exit $LASTEXITCODE

