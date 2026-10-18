$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\src\Game\NPC\KinopioAstro.cpp'
$output=Join-Path $n 'src\generated\KinopioAstroRuntime.cpp'
$text=[System.IO.File]::ReadAllText($source)
$pattern='u32 JKRArchive::getExpandedResSize\(const void\* pResource\) const\s*\{\s*return getResSize\(pResource\);\s*\}'
$count=[regex]::Matches($text,$pattern).Count
if($count -ne 1){throw "KinopioAstro expected one JKRArchive compatibility definition, found $count"}
# The port supplies this generic JKRArchive method in native_jkr_archive_port.cpp.
# Keep the original KinopioAstro implementation but remove its duplicate shim.
$text=[regex]::Replace($text,$pattern,'')
[System.IO.File]::WriteAllText($output,$text,[System.Text.UTF8Encoding]::new($false))
