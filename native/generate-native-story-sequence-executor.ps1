$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\System\StorySequenceExecutor.cpp'
$output = Join-Path $n 'src\generated\StorySequenceExecutorRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$old = 'tryNextDemoInfo() != nullptr'
$count = [regex]::Matches($text, [regex]::Escape($old)).Count
if ($count -ne 2) { throw "Expected two bool/null comparisons in $source; found $count" }
$text = $text.Replace($old, 'tryNextDemoInfo()')
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
