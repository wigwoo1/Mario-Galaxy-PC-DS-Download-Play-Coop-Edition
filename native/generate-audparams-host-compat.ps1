$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\src\Game\AudioLib\AudParams.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='const int aramAddr = nullptr;'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'AudParams expected one integer zero-address sentinel'}
$text=$text.Replace($old,'const int aramAddr = 0;')
$out=Join-Path $n 'src\generated\AudParamsRuntime.cpp'
[System.IO.File]::WriteAllText($out,$text,[System.Text.UTF8Encoding]::new($false))
