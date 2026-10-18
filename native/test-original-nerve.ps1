$ErrorActionPreference='Stop'
$PSNativeCommandUseErrorActionPreference = $false
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$n\build\original-nerve-tests-cache"
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\original_nerve_tests.cpp" "$p\Petari\src\Game\LiveActor\Spine.cpp" "$p\Petari\src\Game\LiveActor\Nerve.cpp" "$p\Petari\src\Game\LiveActor\ActorStateKeeper.cpp" "$p\Petari\src\Game\System\NerveExecutor.cpp" "$p\Petari\src\Game\Util\NerveUtil.cpp" "$n\src\generated\MathUtilScalar.cpp" "$p\Petari\src\JSystem\JMath\JMATrigonometric.cpp" -o "$n\build\original-nerve-tests.exe" *> "$n\build\original-nerve-tests.log"
    $compileExit = $LASTEXITCODE
} finally { $ErrorActionPreference = $previousErrorActionPreference }
if($compileExit){Get-Content "$n\build\original-nerve-tests.log" -Tail 30;exit $compileExit}
& "$n\build\original-nerve-tests.exe"
exit $LASTEXITCODE


