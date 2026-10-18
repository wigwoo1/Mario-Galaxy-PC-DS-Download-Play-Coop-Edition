$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent

$logoSource = Join-Path $p 'Petari\src\Game\Screen\LogoFader.cpp'
$logoOutput = Join-Path $n 'src\generated\LogoFaderRuntime.cpp'
$logo = [System.IO.File]::ReadAllText($logoSource)
$oldAlpha = 'GXColor fillColor = {0, 0, 0, mRate * 255.0f};'
$newAlpha = 'GXColor fillColor = {0, 0, 0, static_cast<u8>(mRate * 255.0f)};'
if (-not $logo.Contains($oldAlpha)) { throw "Expected LogoFader alpha initializer was not found in $logoSource" }
[System.IO.File]::WriteAllText($logoOutput, $logo.Replace($oldAlpha, $newAlpha), [System.Text.UTF8Encoding]::new($false))

$isbnSource = Join-Path $p 'Petari\src\Game\Screen\IsbnManager.cpp'
$isbnOutput = Join-Path $n 'src\generated\IsbnManagerRuntime.cpp'
$isbn = [System.IO.File]::ReadAllText($isbnSource)
if (-not $isbn.Contains('#include <wstring.h>')) { throw "Expected legacy wstring.h include was not found in $isbnSource" }
[System.IO.File]::WriteAllText($isbnOutput, $isbn.Replace('#include <wstring.h>', '#include <wchar.h>'), [System.Text.UTF8Encoding]::new($false))
