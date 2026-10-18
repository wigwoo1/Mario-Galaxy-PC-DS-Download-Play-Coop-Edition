$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$out=Join-Path $n 'src\generated\MapRideCompat'
New-Item -ItemType Directory -Force $out | Out-Null
$legacyRandom='MR::getRandom\(\s*(-?\d+)[lL]\s*,\s*(-?\d+)[lL]\s*\)'
$legacyAnimation='MR::startBckPlayer\((\s*"[^"\r\n]*"\s*),\s*(-?\d+)[lL](\s*\))'
$unitDirectories=@{SeaGull='Map';FileSelector='Map';SurfRay='Ride';SwingRope='Ride';RaceRail='Map';EffectObjGravityDust='Effect'}
foreach($unit in $unitDirectories.Keys){
    $directory=$unitDirectories[$unit]
    $source=Join-Path $p "Petari\src\Game\$directory\$unit.cpp"
    if(!(Test-Path -LiteralPath $source)){throw "Missing original $directory source $unit"}
    $text=[System.IO.File]::ReadAllText($source)
    if($unit -in @('SeaGull','FileSelector')){
        $matches=[regex]::Matches($text,$legacyRandom).Count
        if($matches -lt 1){throw "$unit expected at least one long-integer MR::getRandom call"}
        $text=[regex]::Replace($text,$legacyRandom,'MR::getRandom(s32($1), s32($2))')
    }
    if($unit -in @('SurfRay','SwingRope','RaceRail')){
        $matches=[regex]::Matches($text,$legacyAnimation).Count
        if($matches -lt 1){throw "$unit expected at least one long-integer MR::startBckPlayer call"}
        $text=[regex]::Replace($text,$legacyAnimation,'MR::startBckPlayer($1, s32($2)$3')
    }
    if($unit -eq 'SwingRope'){
        $old='new JUTTexture(MR::loadTexFromArc("SwingRope.arc", "SwingRope.bti"), 0)'
        if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'SwingRope expected one numeric texture palette-mode argument'}
        $text=$text.Replace($old,'new JUTTexture(MR::loadTexFromArc("SwingRope.arc", "SwingRope.bti"), u8(0))')
    }
    if($unit -eq 'EffectObjGravityDust'){
        $old='return &(TVec3f(0.0f, 500.0f * mScale.y, 0.0f));'
        if([regex]::Matches($text,[regex]::Escape($old)).Count -ne 1){throw 'EffectObjGravityDust expected exactly one temporary clipping-offset expression'}
        $text=$text.Replace($old,'static thread_local TVec3f offset; offset.set(0.0f, 500.0f * mScale.y, 0.0f); return &offset;')
    }
    if($unit -eq 'SurfRay'){
        $open="case 7:`r`n    case 11:`r`n"
        if(!$text.Contains($open)){$open="case 7:`n    case 11:`n"}
        if(!$text.Contains($open)){throw 'SurfRay expected case 7/11 switch labels'}
        $text=$text.Replace($open,$open.Replace('case 11:','case 11: {'))
        $close="        break;`r`n    case 13:"
        if(!$text.Contains($close)){$close="        break;`n    case 13:"}
        if(!$text.Contains($close)){throw 'SurfRay expected closing break before case 13'}
        $text=$text.Replace($close,$close.Replace('    case 13:','    }`r`n    case 13:').Replace('`r`n',[Environment]::NewLine))
    }
    [System.IO.File]::WriteAllText((Join-Path $out "$unit.cpp"),$text,[System.Text.UTF8Encoding]::new($false))
}
