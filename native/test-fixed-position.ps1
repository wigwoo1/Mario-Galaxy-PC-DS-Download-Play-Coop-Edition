$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-placement-matrix.ps1"
& "$PSScriptRoot\generate-parts-model.ps1"
$s=Get-Content "$PSScriptRoot\src\generated\FixedPositionKernel.cpp" -Raw
$start=$s.IndexOf('FixedPosition::FixedPosition(MtxPtr');$end=$s.IndexOf('FixedPosition::FixedPosition(const LiveActor* pActor, const char* pResName')
if($start -lt 0 -or $end -le $start){throw 'FixedPosition transform anchors missing'}
$prefix="#include <JSystem/JGeometry/TMatrix.hpp>`n#include <math_types.hpp>`n#include <Game/Util/FixedPosition.hpp>`n#include <Game/Util/MtxUtil.hpp>`n"
Set-Content "$PSScriptRoot\src\generated\FixedPositionTransformFixture.cpp" ($prefix+$s.Substring($start,$end-$start))
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp" "$n\src\fixed_position_tests.cpp" "$n\src\generated\FixedPositionTransformFixture.cpp" "$n\src\generated\CollisionMath.cpp" "$n\src\sdk_matrix_port.cpp" "$n\src\generated\PlacementMatrixKernel.cpp" "$n\build\actor-runtime-probe\JMATrigonometric.obj" -o "$n\build\fixed-position-tests.exe" *> "$n\build\fixed-position-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\fixed-position-tests.log" -Tail 20;exit 1}
& "$n\build\fixed-position-tests.exe"
exit $LASTEXITCODE









