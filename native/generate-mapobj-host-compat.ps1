$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$out=Join-Path $n 'src\generated\MapObjCompat'
New-Item -ItemType Directory -Force $out | Out-Null
$units=@('AssemblyBlock','BrightObj','ClipAreaDrop','ClipAreaMovable','CocoNut','CrystalCage','CrystalCageMoving','DriftWood','EarthenPipe','ElectricBall','ElectricRailHolder','FirePressureRadiate','GeneralMapParts','GreenCaterpillarBig','IceVolcanoUpDownPlane','MapPartsBreaker','MapPartsRailGuideDrawer','MapPartsRailPosture','MapPartsRailRotator','MapPartsRotator','MarioLauncherAttractor','MiniatureGalaxy','Note','OceanWaveFloater','PhantomShipBoxFloater','PlantGroup','PowerStar','PunchBox','QuestionCoin','Rock','RockCreator','Sandstorm','Shellfish','SnowFloor','SnowFloorTile','SpringJetWater','SpringWaterFloaterSpot','TrampleStar','TreasureBoxCracked','WaterfallCaveCover','WaterPressureBullet')
$legacyRandom='MR::getRandom\(\s*((?:-?\d+)[lL])\s*,\s*((?:-?\d+)[lL])\s*\)'
foreach($unit in $units){
    $source=Join-Path $p "Petari\src\Game\MapObj\$unit.cpp"
    if(!(Test-Path -LiteralPath $source)){throw "Missing MapObj source $unit"}
    $text=[System.IO.File]::ReadAllText($source)
    if($unit -in @('Rock','RockCreator')){$text=$text.Replace('PI_180','0.017453292f')}
    if($unit -eq 'WaterfallCaveCover'){$text=$text.Replace('FLOAT_MAX','3.4028235e38f')}
    if($unit -in @('AssemblyBlock','DriftWood','MiniatureGalaxy','PhantomShipBoxFloater','PlantGroup','PunchBox','SpringWaterFloaterSpot')){
        $count=[regex]::Matches($text,$legacyRandom).Count
        if($count -lt 1){throw "$unit expected integer-literal getRandom adaptation"}
        $text=[regex]::Replace($text,$legacyRandom,'MR::getRandom(s32($1), s32($2))')
    }
    if($unit -in @('EarthenPipe','PowerStar','WaterPressureBullet')){
        $count=[regex]::Matches($text,'MR::startBckPlayer\([^;\r\n]*,\s*-?\d+[lL]\s*\)').Count
        if($count -lt 1){throw "$unit expected integer startBckPlayer overload"}
        $text=[regex]::Replace($text,'(MR::startBckPlayer\([^,;\r\n]*,\s*)(-?\d+)[lL](\s*\))','$1s32($2)$3')
    }
    switch($unit){
        'CrystalCage' { $old='MR::setBinderOffsetVec(this, &_110, nullptr);'; if(!$text.Contains($old)){throw 'CrystalCage bool sentinel missing'}; $text=$text.Replace($old,'MR::setBinderOffsetVec(this, &_110, false);') }
        'MarioLauncherAttractor' { $old='initModelManagerWithAnm("MarioLauncherAttractor", nullptr, nullptr);'; if(!$text.Contains($old)){throw 'MarioLauncherAttractor bool sentinel missing'}; $text=$text.Replace($old,'initModelManagerWithAnm("MarioLauncherAttractor", nullptr, false);') }
        'MapPartsRailPosture' { $old='|| isPostureTypeRailDirRailUseShadowGravity)'; if(!$text.Contains($old)){throw 'MapPartsRailPosture predicate call missing'}; $text=$text.Replace($old,'|| isPostureTypeRailDirRailUseShadowGravity())') }
        'Sandstorm' { $old='if (isSunakazeKun)'; if(!$text.Contains($old)){throw 'Sandstorm predicate call missing'}; $text=$text.Replace($old,'if (isSunakazeKun())') }
        'PlantGroup' {
            foreach($pair in @(
                @('const TVec3f* pUp = &(rGravity * 100.0f);','const TVec3f up = rGravity * 100.0f;`r`n        const TVec3f* pUp = &up;'),
                @('const TVec3f* pRay = &(rGravity * ::sCheckLineLength);','const TVec3f ray = rGravity * ::sCheckLineLength;`r`n        const TVec3f* pRay = &ray;')
            )){if(!$text.Contains($pair[0])){throw "PlantGroup temporary adapter missing $($pair[0])"};$text=$text.Replace($pair[0],$pair[1].Replace('`r`n',"`r`n"))}
        }
        'ElectricBall' {
            $text=[regex]::Replace($text,'std::for_each\(mBalls\.begin\(\),\s*mBalls\.end\(\),[\s\S]*?mBaseMtx\)\);','std::for_each(mBalls.begin(), mBalls.end(), [this](Ball& ball) { ball.updatePosition(mBaseMtx); });')
            $text=[regex]::Replace($text,'std::for_each_array\(mBalls\.begin\(\),\s*mBalls\.end\(\),\s*std::binder2nd\s*<\s*std::mem_fun1_t\s*<\s*void,\s*Ball,\s*LiveActor\*\s*>\s*,\s*LiveActor\*\s*>\s*\(std::mem_func\(&Ball::init\),\s*this\)\s*\);','std::for_each_array(mBalls.begin(), mBalls.end(), [this](Ball& ball) { ball.init(this); });')
            $text=$text.Replace('std::for_each_array(mBalls.begin(), mBalls.end(), std::mem_func(&Ball::activate));','std::for_each_array(mBalls.begin(), mBalls.end(), [](Ball& ball) { ball.activate(); });')
            $text=$text.Replace('std::for_each_array','std::for_each')
            if($text -match 'binder2nd|mem_fun1|mem_func'){throw 'ElectricBall legacy callback conversion incomplete'}
        }
        'GeneralMapParts' {
            $text=$text.Replace('std::mem_fun(&MapPartsFunction::end)','[](MapPartsFunction* function) { function->end(); }')
            $text=[regex]::Replace($text,'std::binder2nd\s*<\s*std::mem_fun1_t\s*<\s*bool,\s*MapPartsFunction,\s*u32\s*>\s*,\s*u32\s*>\s*\(std::mem_func\(&MapPartsFunction::receiveMsg\),\s*msg\)','[msg](MapPartsFunction* function) { return function->receiveMsg(msg); }')
            if($text -match 'binder2nd|mem_fun|mem_func'){throw 'GeneralMapParts legacy callback conversion incomplete'}
        }
        'MapPartsRailGuideDrawer' {
            $text=$text.Replace('std::mem_fun(&LiveActor::appear)','[](LiveActor* actor) { actor->appear(); }').Replace('std::mem_fun(&LiveActor::kill)','[](LiveActor* actor) { actor->kill(); }')
            if($text -match 'mem_fun'){throw 'MapPartsRailGuideDrawer legacy callback conversion incomplete'}
        }
        'MapPartsBreaker' { $old='f32 TVec2f::squared('; if(!$text.Contains($old)){throw 'MapPartsBreaker member extension missing'}; $text=$text.Replace($old,'template<> f32 TVec2f::squared(') }
        'CocoNut' { $old='_94.set< f32 >(stack_14);'; if(!$text.Contains($old)){throw 'CocoNut vector set adapter missing'}; $text=$text.Replace($old,'_94.set(stack_14);') }
        'GreenCaterpillarBig' { $old='MR::blendVec(pOutDir,'; if(!$text.Contains($old)){throw 'GreenCaterpillarBig vector pointer adapter missing'}; $text=$text.Replace($old,'MR::blendVec(reinterpret_cast<Vec*>(pOutDir),') }
        'IceVolcanoUpDownPlane' { $text=[regex]::Replace($text,'JGeometry::TUtil<\s*f32\s*>::epsilon\(\)','3.8146973e-6f'); if($text -match 'JGeometry::TUtil'){throw 'IceVolcanoUpDownPlane epsilon use remains'} }
    }
    [System.IO.File]::WriteAllText((Join-Path $out "$unit.cpp"),$text,[System.Text.UTF8Encoding]::new($false))
}
