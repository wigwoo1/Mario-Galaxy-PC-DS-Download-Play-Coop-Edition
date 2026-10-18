$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\MarioLauncherAttractor.cpp" -Raw
$anchor='initModelManagerWithAnm("MarioLauncherAttractor", nullptr, nullptr);'
if(!$s.Contains($anchor)){throw 'Launcher model initialization anchor missing'}
$s=$s.Replace($anchor,'initModelManagerWithAnm("MarioLauncherAttractor", nullptr, false);')
Set-Content "$n\src\generated\MarioLauncherAttractor.cpp" $s
