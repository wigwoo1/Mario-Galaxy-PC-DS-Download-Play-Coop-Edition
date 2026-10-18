$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\NWC24\NWC24Messenger.cpp'
$output = Join-Path $n 'src\generated\NWC24MessengerRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$oldNull = 'selectTask() == nullptr'
$oldNonNull = 'selectTask() != nullptr'
if ([regex]::Matches($text, [regex]::Escape($oldNull)).Count -ne 1 -or [regex]::Matches($text, [regex]::Escape($oldNonNull)).Count -ne 1) {
    throw "Expected one bool/null check of each kind was not found in $source"
}
$text = $text.Replace($oldNull, '!selectTask()').Replace($oldNonNull, 'selectTask()')
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
