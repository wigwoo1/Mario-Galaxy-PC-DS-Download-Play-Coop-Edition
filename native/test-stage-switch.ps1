$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$original=Get-Content -LiteralPath "$p\Petari\src\Game\Map\StageSwitch.cpp" -Raw
$boundary=$original.IndexOf('SwitchIdInfo::SwitchIdInfo')
if($boundary -lt 0){throw 'Missing original switch bit kernel anchor'}
Set-Content -LiteralPath "$n\src\generated\StageSwitchBitKernel.cpp" -Value $original.Substring(0,$boundary)
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\stage_switch_tests.cpp" "$n\src\generated\StageSwitchBitKernel.cpp" -o "$n\build\stage-switch-tests.exe" *> "$n\build\stage-switch-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\stage-switch-tests.log" -Tail 20;exit 1}
& "$n\build\stage-switch-tests.exe"
exit $LASTEXITCODE


