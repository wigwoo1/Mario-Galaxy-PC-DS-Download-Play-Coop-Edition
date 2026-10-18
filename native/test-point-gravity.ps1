$n=$PSScriptRoot;$p=Split-Path $n -Parent
function ExtractIK([string]$s,[string]$sig){$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing IK anchor: $sig"};$b=$s.IndexOf('{',$a);$depth=1;$e=$b+1;while($depth){if($s[$e] -eq '{'){$depth++}elseif($s[$e] -eq '}'){$depth--};$e++};$s.Substring($a,$e-$a)}
$math=Get-Content "$p\Petari\src\Game\Util\MathUtil.cpp" -Raw
$mtx=Get-Content "$p\Petari\src\Game\Util\MtxUtil.cpp" -Raw
Set-Content "$n\src\generated\IKMathFixture.cpp" ('#include <Game/Util/MathUtil.hpp>'+[Environment]::NewLine+'namespace MR {'+[Environment]::NewLine+(ExtractIK $math '    bool normalizeOrZero(TVec3f*')+[Environment]::NewLine+(ExtractIK $mtx '    void extractMtxTrans(')+[Environment]::NewLine+'}')
$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-placement-matrix.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp" "$n\src\point_gravity_tests.cpp" "$n\src\generated\CollisionMath.cpp" "$p\Petari\src\Game\Gravity\PlanetGravity.cpp" "$p\Petari\src\Game\Gravity\PointGravity.cpp" "$n\src\generated\IKMathFixture.cpp" "$n\src\collision_math_host.cpp" "$n\src\sdk_matrix_port.cpp" "$n\src\generated\PlacementMatrixKernel.cpp" "$n\build\actor-runtime-probe\JMATrigonometric.obj" -o "$n\build\point-gravity-tests.exe" *> "$n\build\point-gravity-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\point-gravity-tests.log" -Tail 20;exit 1}
& "$n\build\point-gravity-tests.exe"
exit $LASTEXITCODE









