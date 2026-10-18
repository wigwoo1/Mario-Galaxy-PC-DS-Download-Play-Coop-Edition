$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Screen\GalaxyNamePlate.cpp'
$output = Join-Path $n 'src\generated\GalaxyNamePlateRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$runtimeInclude = '#include <runtime.h>'
$projectedPosition = 'setTrans(vec);'
if ([regex]::Matches($text, [regex]::Escape($runtimeInclude)).Count -ne 1) {
    throw "Expected one Metrowerks runtime include in $source"
}
if ([regex]::Matches($text, [regex]::Escape($projectedPosition)).Count -ne 1) {
    throw "Expected one projected GalaxyNamePlate position in $source"
}
if ($text -match '__cvt_fp2unsigned|__div2[ui]|__mod2[ui]|__shl2i|__shr2u|__cvt_dbl_ull') {
    throw "GalaxyNamePlate now uses a Metrowerks runtime helper; review host adaptation in $source"
}
$text = $text.Replace($runtimeInclude, '// Host build: this file uses no Metrowerks runtime entry points.')
$text = $text.Replace($projectedPosition, 'setTrans(TVec2f(vec.x, vec.y));')
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
