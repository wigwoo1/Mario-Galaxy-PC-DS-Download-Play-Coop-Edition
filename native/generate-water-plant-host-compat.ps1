$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\src\Game\Map\WaterPlant.cpp'
$output=Join-Path $n 'src\generated\WaterPlantRuntime.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old=', nullptr);'
$count=[regex]::Matches($text,[regex]::Escape($old)).Count
if($count -ne 4){throw "WaterPlant expected 4 palette-index nullptr arguments, found $count"}
$text=$text.Replace($old,', 0);')
[System.IO.File]::WriteAllText($output,$text,[System.Text.UTF8Encoding]::new($false))
