$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\src\Game\Util\GravityUtil.cpp'
$out=Join-Path $n 'src\generated\GravityCompat'
New-Item -ItemType Directory -Force $out | Out-Null
$text=[System.IO.File]::ReadAllText($source)
$hostDeclarations=[regex]::Matches($text,'u32 host').Count
if($hostDeclarations -ne 12){throw "GravityUtil expected 12 32-bit host parameters, found $hostDeclarations"}
$text=$text.Replace('u32 host','std::uintptr_t host')
$old='host = (u32)pActor;'
if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'GravityUtil expected one Wii-width actor pointer cast'}
$text=$text.Replace($old,'host = reinterpret_cast<std::uintptr_t>(pActor);')
[System.IO.File]::WriteAllText((Join-Path $out 'GravityUtil.cpp'),$text,[System.Text.UTF8Encoding]::new($false))
