$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Map\SphereSelector.cpp'
$output = Join-Path $n 'src\generated\SphereSelectorRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$distance = '_A8.squareDist(MR::getStarPointerScreenPositionOrEdge(0))'
$random = 'MR::getRandom(0L, 4L)'
if ([regex]::Matches($text, [regex]::Escape($distance)).Count -ne 1 -or
    [regex]::Matches($text, [regex]::Escape($random)).Count -ne 2) {
    throw "Expected one host TVec2 distance expression and two bounded random calls in $source"
}
$text = $text.Replace($distance, '_A8.subOperatorInline(MR::getStarPointerScreenPositionOrEdge(0)).squared()')
$text = $text.Replace($random, 'MR::getRandom(s32(0), s32(4))')
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
