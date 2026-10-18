$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$out=Join-Path $n 'src\generated\NPCCompat'
New-Item -ItemType Directory -Force $out | Out-Null
$numericRandom='MR::getRandom\(\s*((?:0[xX][0-9A-Fa-f]+|\d+))[lL]?\s*,\s*((?:0[xX][0-9A-Fa-f]+|\d+))[lL]?\s*\)'
foreach($unit in @('Rosetta','RosettaReading','StrayTico','Syati','Tico','TicoRail')){
    $source=Join-Path $p "Petari\src\Game\NPC\$unit.cpp"
    $text=[System.IO.File]::ReadAllText($source)
    if($unit -in @('Rosetta','Syati','Tico','TicoRail')){
        $matches=[regex]::Matches($text,$numericRandom).Count
        if($matches -lt 1){throw "$unit expected at least one integer getRandom call"}
        $text=[regex]::Replace($text,$numericRandom,'MR::getRandom(s32($1), s32($2))')
    }
    if($unit -eq 'RosettaReading'){
        if($text -match '#include\s+"math_types\.hpp"'){throw 'RosettaReading already includes the zero-vector declaration'}
        $text="#include `"math_types.hpp`"`r`n$text"
    }
    if($unit -eq 'Syati'){
        $old='initEffectKeeper(1, nullptr, nullptr);'
        if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Syati expected one null-to-bool effect keeper site'}
        $text=$text.Replace($old,'initEffectKeeper(1, nullptr, false);')
        $old='_20(&pSyati->mMarioMoveLocalOffsetVec)'
        if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'Syati expected one Wii Vec-compatible local offset pointer'}
        $text=$text.Replace($old,'_20(reinterpret_cast<Vec *>(&pSyati->mMarioMoveLocalOffsetVec))')
    }
    if($unit -eq 'TicoRail'){
        $old='MR::blendVec(&_8C,'
        if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 2){throw 'TicoRail expected two Wii-layout Vec blend destinations'}
        $text=$text.Replace($old,'MR::blendVec(reinterpret_cast<Vec*>(&_8C),')
    }
    [System.IO.File]::WriteAllText((Join-Path $out "$unit.cpp"),$text,[System.Text.UTF8Encoding]::new($false))
}
