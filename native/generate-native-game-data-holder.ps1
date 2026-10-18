$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\System\GameDataHolder.cpp'
$output = Join-Path $n 'src\generated\GameDataHolderRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$old = 'MR::clamp(value + points, 0l, 20l)'
$new = 'MR::clamp(static_cast<s32>(value + points), 0, 20)'
if (-not $text.Contains($old)) { throw "Expected GameDataHolder clamp expression was not found in $source" }
$text = $text.Replace($old, $new)
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
