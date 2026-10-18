$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\actor-runtime-probe"
$unit='XanimeResource';$source="$n\src\generated\XanimeResource.cpp"
& "$n\generate-xanime-resource.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$line=Get-Content "$n\probe-actor-runtime.ps1"|Where-Object {$_ -like '& *zig.exe*'}
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 20;exit 1}
$results=@(Get-Content "$out\results.json" -Raw|ConvertFrom-Json|Where-Object Unit -ne $unit)
$results += [pscustomobject]@{Unit=$unit;ExitCode=0;Log="$out\$unit.log"}
$results|ConvertTo-Json|Set-Content "$out\results.json"
Write-Output 'Original XanimeResource provider compiled and registered'


