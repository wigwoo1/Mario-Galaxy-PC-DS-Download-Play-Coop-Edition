$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\Screen\GalaxyMap.cpp'
$output = Join-Path $n 'src\generated\GalaxyMapRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$old = 'std::mem_func('
$count = [regex]::Matches($text, [regex]::Escape($old)).Count
if ($count -ne 41) { throw "Expected41 legacy member-function adapters in $source, found$count" }
$text = $text.Replace($old, 'std::mem_fn(')
# Replace removed TR1 binders with direct calls that preserve each icon's
# original draw/calc order and reference the same DrawInfo for the full loop.
foreach ($entry in @(
    @{ Field = 'mDomeIcon'; Class = 'GalaxyMapDomeIcon' },
    @{ Field = 'mIcon'; Class = 'GalaxyMapIcon' },
    @{ Field = 'mCometIcon'; Class = 'GalaxyMapCometIcon' }
)) {
    $pattern = 'std::for_each\(' + $entry.Field + '\.begin\(\),\s*' + $entry.Field + '\.end\(\),\s*std::binder2nd<.*?std::mem_fn\(&' + $entry.Class + '::calcAnimForCapture\),\s*rDrawInfo\)\);'
    if ([regex]::Matches($text, $pattern, [System.Text.RegularExpressions.RegexOptions]::Singleline).Count -ne 1) {
        throw "Missing one calc binder for $($entry.Class) in $source"
    }
    $replacement = 'std::for_each(' + $entry.Field + '.begin(), ' + $entry.Field + '.end(), [&](auto* icon) { icon->calcAnimForCapture(rDrawInfo); });'
    $text = [regex]::Replace($text, $pattern, $replacement, [System.Text.RegularExpressions.RegexOptions]::Singleline)
}
$oldDraw = @(
    'std::for_each(mDomeIcon.begin(), mDomeIcon.end(), std::bind2nd(std::ptr_fun(MR::drawLayoutWithDrawInfoWithoutProjectionSetup), rDrawInfo));',
    'std::for_each(mIcon.begin(), mIcon.end(), std::bind2nd(std::ptr_fun(MR::drawLayoutWithDrawInfoWithoutProjectionSetup), rDrawInfo));',
    'std::for_each(mCometIcon.begin(), mCometIcon.end(), std::bind2nd(std::ptr_fun(MR::drawLayoutWithDrawInfoWithoutProjectionSetup), rDrawInfo));'
)
$newDraw = @(
    'std::for_each(mDomeIcon.begin(), mDomeIcon.end(), [&](auto* icon) { MR::drawLayoutWithDrawInfoWithoutProjectionSetup(icon, rDrawInfo); });',
    'std::for_each(mIcon.begin(), mIcon.end(), [&](auto* icon) { MR::drawLayoutWithDrawInfoWithoutProjectionSetup(icon, rDrawInfo); });',
    'std::for_each(mCometIcon.begin(), mCometIcon.end(), [&](auto* icon) { MR::drawLayoutWithDrawInfoWithoutProjectionSetup(icon, rDrawInfo); });'
)
for ($i = 0; $i -lt $oldDraw.Count; $i++) {
    if ([regex]::Matches($text, [regex]::Escape($oldDraw[$i])).Count -ne 1) { throw "Expected one GalaxyMap draw binder $i in $source" }
    $text = $text.Replace($oldDraw[$i], $newDraw[$i])
}
if ($text.Contains('std::binder2nd') -or $text.Contains('std::bind2nd') -or $text.Contains('std::mem_fun1_t')) {
    throw "A removed TR1 binder remains in the host source copy for $source"
}
if ($text -notmatch '#include\s*<functional>') { $text = "#include <functional>`r`n" + $text }
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
