$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
function New-HostCopy([string]$name, [hashtable[]]$edits) {
    $source = Join-Path $p "Petari\src\Game\Player\$name.cpp"
    $output = Join-Path $n "src\generated\${name}Runtime.cpp"
    $text = [System.IO.File]::ReadAllText($source)
    foreach ($edit in $edits) {
        $count = if ($edit.Regex) { [regex]::Matches($text, $edit.Old).Count } else { [regex]::Matches($text, [regex]::Escape($edit.Old)).Count }
        if ($count -ne $edit.Count) { throw "$name expected $($edit.Count) occurrence(s) of '$($edit.Old)', found $count" }
        if ($edit.Regex) { $text = [regex]::Replace($text, $edit.Old, $edit.New) } else { $text = $text.Replace($edit.Old, $edit.New) }
    }
    [System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
}
New-HostCopy 'MarioActorEye' @(
    @{ Old='MR::getRandom(60L, 360L)'; New='MR::getRandom(static_cast<s32>(60), static_cast<s32>(360))'; Count=2 }
)
New-HostCopy 'MarioActorGravity' @(
    @{ Old='setFrontVecKeepUp(head, 1UL)'; New='setFrontVecKeepUp(head, static_cast<u32>(1))'; Count=1 }
)
New-HostCopy 'MarioActorParts' @(
    @{ Old='_468 == nullptr'; New='_468 == 0'; Count=1 }
)
New-HostCopy 'MarioActorRushMsg' @(
    @{ Old='if (out != nullptr)'; New='if (out)'; Count=1 }
)
New-HostCopy 'MarioActorSensor' @(
    @{ Old='startPadVib(0UL)'; New='startPadVib(static_cast<u32>(0))'; Count=1 }
)
New-HostCopy 'Mario' @(
    @{ Old='MR::normalizeOrZero(&front) == nullptr'; New='!MR::normalizeOrZero(&front)'; Count=1 },
    @{ Old='MR::normalizeOrZero(&headVec) != nullptr'; New='MR::normalizeOrZero(&headVec)'; Count=1 }
)
New-HostCopy 'MarioBee' @(
    @{ Old='setFrontVecKeepUp(front, 1UL)'; New='setFrontVecKeepUp(front, static_cast<u32>(1))'; Count=1 }
)
New-HostCopy 'MarioDamage' @(
    @{ Old='startPadVib(0UL)'; New='startPadVib(static_cast<u32>(0))'; Count=1 }
)
New-HostCopy 'MarioMove' @(
    @{ Old='setFrontVecKeepUp(direction, 15UL)'; New='setFrontVecKeepUp(direction, static_cast<u32>(15))'; Count=1 }
)
New-HostCopy 'MarioShadow' @(
    @{ Old='register '; New=''; Count=1 }
)
New-HostCopy 'MarioSideStep' @(
    @{ Old='changeAnimation\("([^"]*)", 0UL\)'; New='changeAnimation("$1", static_cast<u32>(0))'; Count=2; Regex=$true }
)
New-HostCopy 'MarioStick' @(
    @{ Old='startPadVib(0ul)'; New='startPadVib(static_cast<u32>(0))'; Count=1 }
)
$swimPath = Join-Path $p 'Petari\src\Game\Player\MarioSwim.cpp'
$swimOut = Join-Path $n 'src\generated\MarioSwimRuntime.cpp'
$swimText = [System.IO.File]::ReadAllText($swimPath)
if ($swimText -match '#include\s*["<]Native/AngleConversion.hpp') { throw 'MarioSwim already includes host angle conversions' }
$swimPrefix = '#include "Native/AngleConversion.hpp"' + "`r`n"
[System.IO.File]::WriteAllText($swimOut, $swimPrefix + $swimText, [System.Text.UTF8Encoding]::new($false))
New-HostCopy 'MarioTeresa' @(
    @{ Old='MR::getRandom(60L, 180L)'; New='MR::getRandom(static_cast<s32>(60), static_cast<s32>(180))'; Count=2 },
    @{ Old='MR::getRandom(90L, 240L)'; New='MR::getRandom(static_cast<s32>(90), static_cast<s32>(240))'; Count=1 }
)
New-HostCopy 'MarioWall' @(
    @{ Old='startPadVib(0UL)'; New='startPadVib(static_cast<u32>(0))'; Count=1 }
)
$warpPath = Join-Path $p 'Petari\src\Game\Player\MarioWarp.cpp'
$warpOut = Join-Path $n 'src\generated\MarioWarpRuntime.cpp'
$warpText = [System.IO.File]::ReadAllText($warpPath).Replace("`r`n", "`n")
$decl = '    TVec3< f32 > TVec3< f32 >::operator*(f32) const NO_INLINE;'
if ([regex]::Matches($warpText, [regex]::Escape($decl)).Count -ne 1) { throw 'MarioWarp template declaration anchor changed' }
$warpText = $warpText.Replace($decl + "`n", '')
$case3 = '    case 3:\n        MR::endGlobalEventCamera\("([^"]*)", -1, true\);'
if ([regex]::Matches($warpText, $case3).Count -ne 1) { throw 'MarioWarp case 3 anchor changed' }
$warpText = [regex]::Replace($warpText, $case3, '    case 3: {' + "`n" + '        MR::endGlobalEventCamera("$1", -1, true);')
$case3End = '        player->_430 = 0;' + "`n" + '        break;' + "`n" + '    case 1:'
if ([regex]::Matches($warpText, [regex]::Escape($case3End)).Count -ne 1) { throw 'MarioWarp case 3 end anchor changed' }
$warpText = $warpText.Replace($case3End, ('        player->_430 = 0;' + "`n" + '        break;' + "`n" + '    }' + "`n" + '    case 1:'))
[System.IO.File]::WriteAllText($warpOut, $warpText, [System.Text.UTF8Encoding]::new($false))
$effectPath = Join-Path $p 'Petari\src\Game\Player\MarioEffect.cpp'
$effectOut = Join-Path $n 'src\generated\MarioEffectRuntime.cpp'
$effectText = [System.IO.File]::ReadAllText($effectPath)
$addOld = '_BA4->add(cMaterialEffectTable[i].mName, reinterpret_cast< u32 >(&cMaterialEffectTable[i]), false);'
if ([regex]::Matches($effectText, [regex]::Escape($addOld)).Count -ne 1) { throw 'MarioEffect table key anchor changed' }
$effectText = $effectText.Replace($addOld, '_BA4->add(cMaterialEffectTable[i].mName, static_cast<u32>(i), false);')
$searchOld = "    MaterialEffectEntry* entry = nullptr;`r`n    _BA4->search(pName, reinterpret_cast< u32* >(&entry));"
$searchNew = "    u32 entryIndex = 0;`r`n    _BA4->search(pName, &entryIndex);`r`n    MaterialEffectEntry* entry = &cMaterialEffectTable[entryIndex];"
if ([regex]::Matches($effectText, [regex]::Escape($searchOld)).Count -ne 2) { throw 'MarioEffect table lookup anchors changed' }
$effectText = $effectText.Replace($searchOld, $searchNew)
[System.IO.File]::WriteAllText($effectOut, $effectText, [System.Text.UTF8Encoding]::new($false))
$jumpPath = Join-Path $p 'Petari\src\Game\Player\MarioJump.cpp'
$jumpOut = Join-Path $n 'src\generated\MarioJumpRuntime.cpp'
$jumpText = [System.IO.File]::ReadAllText($jumpPath)
$jumpAnchor = 'namespace {' + "`r`n" + '    f32 cDropFrontSpeed = 2.0f;' + "`r`n" + '};  // namespace'
if ([regex]::Matches($jumpText, [regex]::Escape($jumpAnchor)).Count -ne 1) { throw 'MarioJump sidecar insertion anchor changed' }
$sidecar = @'
namespace {
    f32 cDropFrontSpeed = 2.0f;
    std::unordered_map<u32, HitSensor*> sTornadoCenteringSensors;
    u32 sNextTornadoCenteringSensorId = 1;

    u32 registerTornadoCenteringSensor(HitSensor* pSensor) {
        while (sNextTornadoCenteringSensorId == 0 || sTornadoCenteringSensors.find(sNextTornadoCenteringSensorId) != sTornadoCenteringSensors.end()) {
            ++sNextTornadoCenteringSensorId;
        }
        const u32 id = sNextTornadoCenteringSensorId++;
        sTornadoCenteringSensors.emplace(id, pSensor);
        return id;
    }

    HitSensor* findTornadoCenteringSensor(u32 id) {
        const auto it = sTornadoCenteringSensors.find(id);
        return it == sTornadoCenteringSensors.end() ? nullptr : it->second;
    }

    void eraseTornadoCenteringSensor(u32 id) {
        sTornadoCenteringSensors.erase(id);
    }
};  // namespace
'@
$jumpText = $jumpText.Replace($jumpAnchor, $sidecar)
$jumpText = $jumpText.Replace('#include "Game/Enemy/KariKariDirector.hpp"', '#include "Game/Enemy/KariKariDirector.hpp"' + "`r`n" + '#include <unordered_map>')
$oldStart = '    _A38 = reinterpret_cast< u32 >(pSensor);'
if ([regex]::Matches($jumpText, [regex]::Escape($oldStart)).Count -ne 1) { throw 'MarioJump sensor handle write anchor changed' }
$jumpText = $jumpText.Replace($oldStart, '    _A38 = registerTornadoCenteringSensor(pSensor);')
$oldSensor = '        TVec3f sensorOffset(reinterpret_cast< HitSensor* >(_A38)->mHost->mPosition - mPosition);'
if ([regex]::Matches($jumpText, [regex]::Escape($oldSensor)).Count -ne 1) { throw 'MarioJump sensor handle read anchor changed' }
$newSensor = @'
        HitSensor* pSensor = findTornadoCenteringSensor(_A38);
        if (pSensor == nullptr || pSensor->mHost == nullptr) {
            eraseTornadoCenteringSensor(_A38);
            _A34 = 0;
            return false;
        }
        TVec3f sensorOffset(pSensor->mHost->mPosition - mPosition);
'@
$jumpText = $jumpText.Replace($oldSensor, $newSensor.TrimEnd())
$completionOld = '        if (_A34 == 0) {' + "`r`n" + '            return false;'
$completionNew = '        if (_A34 == 0) {' + "`r`n" + '            eraseTornadoCenteringSensor(_A38);' + "`r`n" + '            return false;'
if ([regex]::Matches($jumpText, [regex]::Escape($completionOld)).Count -ne 1) { throw 'MarioJump sensor handle completion anchor changed' }
$jumpText = $jumpText.Replace($completionOld, $completionNew)
[System.IO.File]::WriteAllText($jumpOut, $jumpText, [System.Text.UTF8Encoding]::new($false))
