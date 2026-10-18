$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-placement-matrix.ps1"
& "$PSScriptRoot\generate-camera-collision-math.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp" "$n\src\camera_collision_math_tests.cpp" "$n\src\generated\CollisionMath.cpp" "$n\src\sdk_matrix_port.cpp" "$n\src\generated\PlacementMatrixKernel.cpp" "$n\src\generated\CameraCollisionMath.cpp" "$n\src\generated\CameraNormalizeFixture.cpp" "$n\src\generated\Vector2NormalizeKernel.cpp" "$n\src\generated\MathUtilScalar.cpp" "$n\build\actor-runtime-probe\JMATrigonometric.obj" -o "$n\build\camera-collision-math-tests.exe" *> "$n\build\camera-collision-math-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\camera-collision-math-tests.log" -Tail 20;exit 1}
& "$n\build\camera-collision-math-tests.exe"
exit $LASTEXITCODE













