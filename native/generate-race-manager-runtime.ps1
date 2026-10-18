$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Map\RaceManager.cpp'
$output = Join-Path $n 'src\generated\RaceManagerRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$old = 'std::mem_func('
$count = [regex]::Matches($text, [regex]::Escape($old)).Count
if ($count -ne 6) { throw "Expected 6 legacy member-function adapters in $source, found $count" }
$text = $text.Replace($old, 'std::mem_fn(')
if ($text -notmatch '#include\s*<functional>') { $text = "#include <functional>`r`n" + $text }
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
