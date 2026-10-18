$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Screen\GalaxyMapSelectButton.cpp'
$output = Join-Path $n 'src\generated\GalaxyMapSelectButtonRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$old = 'mPaneCtrl->mIsSelected != nullptr'
if ([regex]::Matches($text, [regex]::Escape($old)).Count -ne 1) {
    throw "Expected one bool/null compatibility expression in $source"
}
$text = $text.Replace($old, 'mPaneCtrl->mIsSelected')
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
