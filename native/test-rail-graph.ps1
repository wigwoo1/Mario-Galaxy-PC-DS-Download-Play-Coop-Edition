$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$out = "$n\build\rail-graph-tests.exe"
$env:ZIG_GLOBAL_CACHE_DIR = "$n\build\rail-graph-tests-cache"
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu `
        "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" `
        "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" `
        -include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h" `
        -include "$n\include-runtime\nw4r\ut\Color.h" `
        "$p\Petari\src\Game\Map\RailGraph.cpp" `
        "$p\Petari\src\Game\Map\RailGraphNode.cpp" `
        "$p\Petari\src\Game\Map\RailGraphEdge.cpp" `
        "$p\Petari\src\Game\Map\RailGraphIter.cpp" `
        "$n\src\rail_graph_tests.cpp" -o $out *> "$n\build\rail-graph-tests.log"
    $compileExit = $LASTEXITCODE
} finally { $ErrorActionPreference = $previousErrorActionPreference }
if ($compileExit) { Get-Content "$n\build\rail-graph-tests.log" -Tail 30; exit $compileExit }
& $out
exit $LASTEXITCODE
