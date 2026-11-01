$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$root=Split-Path $n -Parent
$source=[IO.File]::ReadAllText((Join-Path $root 'Petari\src\Game\Map\Flag.cpp'))
foreach($spec in @(
 @('new Color8[mNumPointsV * (mNumPointsU + 1)]','Native::allocateFlagColors(mNumPointsV,mNumPointsU)'),
 @('new f32[mNumPointsV * (mNumPointsU + 1) * 2]','Native::allocateFlagTexCoords(mNumPointsV,mNumPointsU)'),
 @('void Flag::loadMaterial() const {',('void Flag::loadMaterial() const {'+"`n"+'    Native::refreshFlagVertexArrays(mColors,mTexST,mNumPointsV,mNumPointsU);'))
)){
 if([regex]::Matches($source,[regex]::Escape($spec[0])).Count-ne 1){throw 'Flag resource anchor missing or duplicated'}
 $source=$source.Replace($spec[0],$spec[1])
 if(!$source.Contains($spec[1])){throw 'Flag resource replacement was not emitted completely'}
}
$source='#include <Native/FlagVertexArrays.hpp>'+"`n"+$source
[IO.File]::WriteAllText((Join-Path $n 'src\generated\Flag.cpp'),$source,[Text.UTF8Encoding]::new($false))
