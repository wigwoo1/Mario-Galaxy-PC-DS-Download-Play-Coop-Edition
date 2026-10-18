$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Screen\odh.cpp'
$output = Join-Path $n 'src\generated\ODHCodecRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$oldSize = 'u16 size[2] = {width, height};'
if ([regex]::Matches($text, [regex]::Escape($oldSize)).Count -ne 1) { throw 'Unexpected ODH dimensions conversion site' }
$text = $text.Replace($oldSize, 'u16 size[2] = {static_cast<u16>(width), static_cast<u16>(height)};')
$oldPointer = 'reinterpret_cast< u32 >(pCoefficients)'
if ([regex]::Matches($text, [regex]::Escape($oldPointer)).Count -ne 1) { throw 'Unexpected ODH coefficient alignment check' }
$text = $text.Replace($oldPointer, 'reinterpret_cast< std::uintptr_t >(pCoefficients)')
if ($text -notmatch '#include\s*<cstdint>') { $text = "#include <cstdint>`r`n" + $text }
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
