$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$out="$n\build\actor-runtime-ui"
New-Item -ItemType Directory -Path $out -Force|Out-Null
$units=@(
    @{Name='InformationObserver';Path="$p\Petari\src\Game\Screen\InformationObserver.cpp"},
    @{Name='RumbleCalculator';Path="$p\Petari\src\Game\Util\RumbleCalculator.cpp"}
)
foreach($unit in $units){
    & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections -target x86_64-windows-gnu `
        "-I$n\include-runtime" "-I$n\include" "-I$n\build\camera-pointer-candidate\include" `
        "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" `
        -include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h" -include "$n\include-runtime\nw4r\ut\Color.h" `
        -c $unit.Path -o "$out\$($unit.Name).obj" *> "$out\$($unit.Name).log"
    if($LASTEXITCODE){Get-Content "$out\$($unit.Name).log" -Tail 20;exit $LASTEXITCODE}
}
Write-Output "Compiled $($units.Count) original UI/rumble providers for the host link audit."
