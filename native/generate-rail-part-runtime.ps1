$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Map\RailPart.cpp'
$output = Join-Path $n 'src\generated\RailPartRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$edits = @(
    @{ Old='rPoint1.epsilonEquals(rPoint1Ctrl, 0.1f)'; New='JGeometry::TUtil<f32>::epsilonEquals(rPoint1.x, rPoint1Ctrl.x, 0.1f) && JGeometry::TUtil<f32>::epsilonEquals(rPoint1.y, rPoint1Ctrl.y, 0.1f) && JGeometry::TUtil<f32>::epsilonEquals(rPoint1.z, rPoint1Ctrl.z, 0.1f)'; Count=1 },
    @{ Old='rPoint2.epsilonEquals(rPoint2Ctrl, 0.1f)'; New='JGeometry::TUtil<f32>::epsilonEquals(rPoint2.x, rPoint2Ctrl.x, 0.1f) && JGeometry::TUtil<f32>::epsilonEquals(rPoint2.y, rPoint2Ctrl.y, 0.1f) && JGeometry::TUtil<f32>::epsilonEquals(rPoint2.z, rPoint2Ctrl.z, 0.1f)'; Count=1 },
    @{ Old='C_VECMag(&mCtrlDegree1)'; New='mCtrlDegree1.length()'; Count=1 },
    @{ Old='const register TVec3f& rPos'; New='const TVec3f& rPos'; Count=1 }
)
foreach ($edit in $edits) {
    $count = [regex]::Matches($text, [regex]::Escape($edit.Old)).Count
    if ($count -ne $edit.Count) { throw "RailPart expected $($edit.Count) occurrence(s) of '$($edit.Old)', found $count" }
    $text = $text.Replace($edit.Old, $edit.New)
}
if ($text -notmatch '#include\s*[<"]JSystem/JGeometry/TUtil.hpp') { $text = '#include <JSystem/JGeometry/TUtil.hpp>' + "`r`n" + $text }
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
