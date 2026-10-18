$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\actor-runtime-probe"
$unit='J3DModelLoaderCalcSize';$source="$n\src\generated\J3DModelLoaderCalcSize.cpp"
& "$n\generate-model-loader-size.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$line=Get-Content "$n\probe-actor-runtime.ps1"|Where-Object {$_ -like '& *zig.exe*'}
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 20;exit 1}
$results=@(Get-Content "$out\results.json" -Raw|ConvertFrom-Json|Where-Object Unit -ne $unit)
$results += [pscustomobject]@{Unit=$unit;ExitCode=0;Log="$out\$unit.log"}
$results|ConvertTo-Json|Set-Content "$out\results.json"
Write-Output 'Original J3DModelLoaderCalcSize provider compiled and registered'


