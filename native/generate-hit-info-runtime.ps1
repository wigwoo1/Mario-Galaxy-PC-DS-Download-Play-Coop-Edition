$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Map\HitInfo.cpp'
$output = Join-Path $n 'src\generated\HitInfoRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$signatures = @(
    'Triangle::Triangle() {',
    'void Triangle::fillData(CollisionParts* pParts, u32 index, HitSensor* pSensor) {',
    'bool Triangle::isHostMoved() const {',
    'const TVec3f* Triangle::getNormal(int index) const {',
    'const TVec3f* Triangle::getFaceNormal() const {',
    'void Triangle::calcForceMovePower(TVec3f* a1, const TVec3f& a2) const {',
    'HitInfo::HitInfo() : mParentTriangle(), _60(0.0f), mHitPos(0, 0, 0), _70(0, 0, 0), _7C(0, 0, 0) {'
)
$ranges = [System.Collections.Generic.List[object]]::new()
foreach ($signature in $signatures) {
    $start = $text.IndexOf($signature, [System.StringComparison]::Ordinal)
    if ($start -lt 0 -or $text.IndexOf($signature, $start + $signature.Length, [System.StringComparison]::Ordinal) -ge 0) {
        throw "Expected exactly one duplicate HitInfo definition: $signature"
    }
    $open = $start + $signature.Length - 1
    $depth = 0
    $end = -1
    for ($i = $open; $i -lt $text.Length; $i++) {
        if ($text[$i] -eq '{') { $depth++ }
        elseif ($text[$i] -eq '}') {
            $depth--
            if ($depth -eq 0) { $end = $i + 1; break }
        }
    }
    if ($end -lt 0) { throw "Unclosed duplicate HitInfo definition: $signature" }
    $ranges.Add([pscustomobject]@{ Start=$start; End=$end })
}
foreach ($range in @($ranges | Sort-Object Start -Descending)) {
    $text = $text.Remove($range.Start, $range.End - $range.Start)
}
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
