& "$PSScriptRoot\generate-rumble-channel.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp" "$n\src\original_rumble_channel_tests.cpp" "$n\src\generated\CollisionMath.cpp" "$n\src\sdk_matrix_port.cpp" "$n\src\generated\Vector2NormalizeKernel.cpp" "$n\src\generated\RumbleChannelKernel.cpp" "$n\build\actor-runtime-probe\JMATrigonometric.obj" -o "$n\build\original-rumble-channel-tests.exe" *> "$n\build\original-rumble-channel-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-rumble-channel-tests.log" -Tail 20;exit 1}
& "$n\build\original-rumble-channel-tests.exe"
exit $LASTEXITCODE











