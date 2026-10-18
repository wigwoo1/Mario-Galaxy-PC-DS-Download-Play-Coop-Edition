$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\src\Game\MapObj\LavaFloater.cpp'
$output=Join-Path $n 'src\generated\LavaFloaterRuntime.cpp'
$text=[System.IO.File]::ReadAllText($source)
$old='initEffectKeeper(nullptr, nullptr, false);'
$count=[regex]::Matches($text,[regex]::Escape($old)).Count
if($count -ne 1){throw "LavaFloater expected one null effect-count compatibility site, found $count"}
# The original integer effect-count overload uses zero here; spell it as 0 on the host.
$text=$text.Replace($old,'initEffectKeeper(0, nullptr, false);')
[System.IO.File]::WriteAllText($output,$text,[System.Text.UTF8Encoding]::new($false))
