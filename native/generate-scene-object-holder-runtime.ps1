$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Scene\SceneObjHolder.cpp'
$output = Join-Path $n 'src\generated\SceneObjHolderRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$methods = @(
    @{ Owner='SceneObjHolder'; Name='SceneObjHolder' },
    @{ Owner='SceneObjHolder'; Name='create' },
    @{ Owner='SceneObjHolder'; Name='getObj' },
    @{ Owner='SceneObjHolder'; Name='isExist' },
    @{ Owner='SceneObjHolder'; Name='newEachObj' }
)
$ranges = [System.Collections.Generic.List[object]]::new()
foreach ($method in $methods) {
    $pattern = '(?m)^[ \t]*(?:[\w:<>,*&~]+[ \t]+)*' + [regex]::Escape($method.Owner + '::' + $method.Name) + '\s*\('
    $match = [regex]::Match($text, $pattern)
    if (!$match.Success -or [regex]::Matches($text, $pattern).Count -ne 1) { throw "Expected exactly one duplicate method $($method.Owner)::$($method.Name)" }
    $openParen = $text.IndexOf('(', $match.Index + $match.Length - 1)
    $parenDepth = 0
    $closeParen = -1
    for ($i = $openParen; $i -lt $text.Length; $i++) {
        if ($text[$i] -eq '(') { $parenDepth++ }
        elseif ($text[$i] -eq ')') { $parenDepth--; if ($parenDepth -eq 0) { $closeParen = $i; break } }
    }
    if ($closeParen -lt 0) { throw "Unclosed signature for $($method.Owner)::$($method.Name)" }
    $openBrace = $text.IndexOf('{', $closeParen)
    $braceDepth = 0
    $end = -1
    for ($i = $openBrace; $i -lt $text.Length; $i++) {
        if ($text[$i] -eq '{') { $braceDepth++ }
        elseif ($text[$i] -eq '}') { $braceDepth--; if ($braceDepth -eq 0) { $end = $i + 1; break } }
    }
    if ($end -lt 0) { throw "Unclosed body for $($method.Owner)::$($method.Name)" }
    $ranges.Add([pscustomobject]@{ Start=$match.Index; End=$end })
}
$globalPattern = '(?m)^[ \t]*SceneObjHolder\*\s+getSceneObjHolder\s*\('
$globalMatch = [regex]::Match($text, $globalPattern)
if (!$globalMatch.Success -or [regex]::Matches($text, $globalPattern).Count -ne 1) { throw 'Expected exactly one duplicate MR::getSceneObjHolder implementation' }
$openParen = $text.IndexOf('(', $globalMatch.Index + $globalMatch.Length - 1)
$parenDepth = 0; $closeParen = -1
for ($i=$openParen; $i -lt $text.Length; $i++) { if ($text[$i] -eq '(') { $parenDepth++ } elseif ($text[$i] -eq ')') { $parenDepth--; if ($parenDepth -eq 0) { $closeParen=$i; break } } }
$openBrace = $text.IndexOf('{', $closeParen); $braceDepth=0; $end=-1
for ($i=$openBrace; $i -lt $text.Length; $i++) { if ($text[$i] -eq '{') { $braceDepth++ } elseif ($text[$i] -eq '}') { $braceDepth--; if ($braceDepth -eq 0) { $end=$i+1; break } } }
if ($end -lt 0) { throw 'Unclosed MR::getSceneObjHolder body' }
$ranges.Add([pscustomobject]@{ Start=$globalMatch.Index; End=$end })
foreach ($range in @($ranges | Sort-Object Start -Descending)) { $text = $text.Remove($range.Start, $range.End - $range.Start) }
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
