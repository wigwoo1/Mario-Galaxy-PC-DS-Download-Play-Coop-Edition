$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\src\Game\System\WPadAcceleration.cpp'
$out=Join-Path $n 'src\generated\WPadCompat'
New-Item -ItemType Directory -Force $out | Out-Null
$text=[System.IO.File]::ReadAllText($source)
$old='return JMathInlineVEC::PSVECSquareDistance(&rA, &rB);'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'WPadAcceleration expected exactly one Wii Vec square-distance call'}
$text=$text.Replace($old,'return JMathInlineVEC::PSVECSquareDistance(reinterpret_cast<const Vec*>(&rA), reinterpret_cast<const Vec*>(&rB));')
[System.IO.File]::WriteAllText((Join-Path $out 'WPadAcceleration.cpp'),$text,[System.Text.UTF8Encoding]::new($false))
