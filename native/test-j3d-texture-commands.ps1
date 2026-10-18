$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"

& "$n\generate-gd-fifo.ps1"
$originalTevs=Get-Content -LiteralPath "$p\Petari\src\JSystem\J3DGraphBase\J3DTevs.cpp" -Raw
$defaultMatch=[regex]::Matches($originalTevs,'(?s)J3DTevStageInfo const j3dDefaultTevStageInfo = \{.*?\};')
if($defaultMatch.Count -ne 1){throw 'Original TEV stage default anchor changed'}
Set-Content -LiteralPath "$n\src\generated\TevStageDefaultKernel.cpp" -Encoding UTF8 -Value ('#include <JSystem/J3DGraphBase/J3DTevs.hpp>'+"`r`n"+$defaultMatch[0].Value+[Environment]::NewLine+[regex]::Match($originalTevs,'(?s)J3DTevSwapModeInfo const j3dDefaultTevSwapMode = \{.*?\};').Value)
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DGD.hpp" -include "$n\include-runtime\JSystem\J3DGraphBase\J3DFifo.hpp" "$n\src\j3d_texture_command_tests.cpp" "$n\src\generated\lyt_texMap.cpp" "$n\src\generated\TextureBufferSizeKernel.cpp" "$n\src\gx_test_data.cpp" "$n\src\generated\TevStageDefaultKernel.cpp" "$n\src\generated\TextureObjectLodKernel.cpp" "$n\src\generated\J3DGD.cpp" "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" "$n\src\sdk_cache_port.cpp" "$n\src\gx_array_port.cpp" "$n\src\gx_draw_port.cpp" "$n\src\gx_texture_cache_port.cpp" "$n\src\gx_indirect_port.cpp" "$n\src\gx_tev_port.cpp" "$n\src\generated\GDVertexDescriptor.cpp" -lbcrypt -ld3d11 -ldxgi -ld3dcompiler_47 -luser32 -lgdi32 -o "$n\build\j3d-texture-command-tests.exe" *> "$n\build\j3d-texture-command-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\j3d-texture-command-tests.log" -Tail 20;exit 1}
& "$n\build\j3d-texture-command-tests.exe" @args
exit $LASTEXITCODE





















