$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-texture-object.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu -include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h" -include "$n\include-runtime\nw4r\ut\Color.h" "-I$n\include-runtime" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" "$n\src\generated\LayoutQuadKernel.cpp" "$n\src\generated\FontGlyphEmissionKernel.cpp" "$n\src\generated\NarrowTextWriter.cpp" "$p\Petari\src\nw4r\ut\ut_TextWriterBase.cpp" "$n\src\generated\WideTextWriterKernel.cpp" "$n\src\generated\MessageTagSkipTagProcessor.cpp" "$p\Petari\src\nw4r\ut\ut_TagProcessorBase.cpp" "$n\src\generated\TextureObjectLodKernel.cpp" "$n\src\resource_font_port.cpp" "$n\src\gpu_resources.cpp" "$p\Petari\src\nw4r\ut\ut_Font.cpp" "$p\Petari\src\nw4r\ut\ut_ResFontBase.cpp" "$p\Petari\src\nw4r\ut\ut_CharStrmReader.cpp" "$n\src\gx_draw_port.cpp" "$n\src\gx_indirect_port.cpp" "$n\src\gx_tev_port.cpp" "$n\src\generated\TevColorKernel.cpp" "$n\src\gd_storage.cpp" "$n\src\layout_quad_tests.cpp" -ld3d11 -ldxgi -ld3dcompiler_47 -luser32 -lgdi32 -lbcrypt -o "$n\build\layout-quad-tests.exe" *> "$n\build\layout-quad-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\layout-quad-tests.log" -Tail 18;exit 1}
& "$n\build\layout-quad-tests.exe" "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs"
exit $LASTEXITCODE











