$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\src\Game\Screen\SubMeterLayout.cpp'
$output=Join-Path $n 'src\generated\SubMeterLayoutHost.cpp'
$text=[IO.File]::ReadAllText($source)
$needle='MR::startAnim(this, "Appear", nullptr);'
if(([regex]::Matches($text,[regex]::Escape($needle))).Count -ne 1){throw 'Expected exactly one default-slot startAnim call in original SubMeterLayout'}
$text=$text.Replace($needle,'MR::startAnim(this, "Appear", 0);')
[IO.File]::WriteAllText($output,$text,[System.Text.UTF8Encoding]::new($false))
