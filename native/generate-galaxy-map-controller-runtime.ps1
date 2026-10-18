$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Screen\GalaxyMapController.cpp'
$output = Join-Path $n 'src\generated\GalaxyMapControllerRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$old = 'MR::Functor(this, GalaxyMapController::capture)'
$new = 'MR::Functor(this, &GalaxyMapController::capture)'
if ([regex]::Matches($text, [regex]::Escape($old)).Count -ne 1) {
    throw "Expected one pointer-to-member callback in $source"
}
$text = $text.Replace($old, $new)
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
