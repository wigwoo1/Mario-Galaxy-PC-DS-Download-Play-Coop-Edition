$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Map\CollisionCategorizedKeeper.cpp'
$output = Join-Path $n 'src\generated\CollisionCategorizedKeeperRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$methods = @(
    @{ Owner='CollisionCategorizedKeeper'; Name='~CollisionCategorizedKeeper' },
    @{ Owner='CollisionCategorizedKeeper'; Name='addToGlobal' },
    @{ Owner='CollisionCategorizedKeeper'; Name='addToZone' },
    @{ Owner='CollisionCategorizedKeeper'; Name='checkStrikeBall' },
    @{ Owner='CollisionCategorizedKeeper'; Name='CollisionCategorizedKeeper' },
    @{ Owner='CollisionCategorizedKeeper'; Name='getStrikeInfo' },
    @{ Owner='CollisionCategorizedKeeper'; Name='getZone' },
    @{ Owner='CollisionCategorizedKeeper'; Name='isSphereOverlappingWithBox' },
    @{ Owner='CollisionCategorizedKeeper'; Name='movement' },
    @{ Owner='CollisionCategorizedKeeper'; Name='removeFromGlobal' },
    @{ Owner='CollisionCategorizedKeeper'; Name='removeFromZone' },
    @{ Owner='CollisionParts'; Name='getTrans' },
    @{ Owner='CollisionZone'; Name='addAndUpdateMinMax' },
    @{ Owner='CollisionZone'; Name='addParts' },
    @{ Owner='CollisionZone'; Name='calcMinMaxAndRadius' },
    @{ Owner='CollisionZone'; Name='calcMinMaxAndRadiusIfMoveOuter' },
    @{ Owner='CollisionZone'; Name='CollisionZone' },
    @{ Owner='CollisionZone'; Name='eraseParts' }
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
        elseif ($text[$i] -eq ')') {
            $parenDepth--
            if ($parenDepth -eq 0) { $closeParen = $i; break }
        }
    }
    if ($closeParen -lt 0) { throw "Unclosed signature for $($method.Owner)::$($method.Name)" }
    $openBrace = $text.IndexOf('{', $closeParen)
    $closeBraceDepth = 0
    $end = -1
    for ($i = $openBrace; $i -lt $text.Length; $i++) {
        if ($text[$i] -eq '{') { $closeBraceDepth++ }
        elseif ($text[$i] -eq '}') {
            $closeBraceDepth--
            if ($closeBraceDepth -eq 0) { $end = $i + 1; break }
        }
    }
    if ($end -lt 0) { throw "Unclosed body for $($method.Owner)::$($method.Name)" }
    $ranges.Add([pscustomobject]@{ Start=$match.Index; End=$end })
}
foreach ($range in @($ranges | Sort-Object Start -Descending)) {
    $text = $text.Remove($range.Start, $range.End - $range.Start)
}
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
