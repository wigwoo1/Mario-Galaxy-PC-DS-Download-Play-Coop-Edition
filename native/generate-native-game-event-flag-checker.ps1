$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$source = Join-Path $p 'Petari\src\Game\System\GameEventFlagChecker.cpp'
$output = Join-Path $n 'src\generated\GameEventFlagCheckerRuntime.cpp'
$text = [System.IO.File]::ReadAllText($source)
$replacements = @(
    @('case GameEventFlag::Type_GalaxyOpenStar:\s*(s32 currentPowerStarNum = .*?return powerStarOpenNum <= currentPowerStarNum;)\s*case GameEventFlag::Type_SpecialStar:', "case GameEventFlag::Type_GalaxyOpenStar:`n        {`n        `$1`n        }`n    case GameEventFlag::Type_SpecialStar:"),
    @('case GameEventFlag::Type_EventFlag:\s*(const char\* pRequirement1 = .*?return isOnRequirement1 && isOnRequirement2;)\s*case GameEventFlag::Type_StoryEvent:', "case GameEventFlag::Type_EventFlag:`n        {`n        `$1`n        }`n    case GameEventFlag::Type_StoryEvent:"),
    @('case GameEventFlag::Type_StarPiece:\s*(GameEventFlagAccessor accessor1 = .*?return mDataHolder->getStarPieceNumGivingToTicoSeed\(pFlag->mStarPieceIndex \+ 8\) >= needStarPieceNum;)\s*case GameEventFlag::Type_EventValueIsZero:', "case GameEventFlag::Type_StarPiece:`n        {`n        `$1`n        }`n    case GameEventFlag::Type_EventValueIsZero:"),
    @('case GameEventFlag::Type_EventValueIsZero:\s*(GameEventFlagAccessor accessor2 = .*?return static_cast< u16 >\(mDataHolder->getGameEventValue\(accessor2.getEventValueName\(\)\)\) == 0;)\s*case GameEventFlag::Type_CompleteMarioAndLuigi:', "case GameEventFlag::Type_EventValueIsZero:`n        {`n        `$1`n        }`n    case GameEventFlag::Type_CompleteMarioAndLuigi:")
)
foreach ($replacement in $replacements) {
    $updated = [regex]::Replace($text, $replacement[0], $replacement[1], [System.Text.RegularExpressions.RegexOptions]::Singleline)
    if ($updated -ceq $text) { throw "Expected switch case scope pattern was not found in $source" }
    $text = $updated
}
[System.IO.File]::WriteAllText($output, $text, [System.Text.UTF8Encoding]::new($false))
