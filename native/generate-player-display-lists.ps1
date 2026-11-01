$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$path=Join-Path $n 'src\generated\MarioActorDraw.cpp'
$source=[IO.File]::ReadAllText($path)
foreach($spec in @(
 @('mDL[0] = new (std::align_val_t{32}) u8[0x100];','mDL[0] = Native::allocatePlayerDisplayList(0x100);'),
 @('mDL[1] = new (std::align_val_t{32}) u8[0x100];','mDL[1] = Native::allocatePlayerDisplayList(0x100);'),
 @('pHolder->mDL = new (std::align_val_t{32}) u8[alignedSize];','pHolder->mDL = Native::allocatePlayerDisplayList(alignedSize);')
)){
 if(!$source.Contains($spec[1])){
  if([regex]::Matches($source,[regex]::Escape($spec[0])).Count-ne 1){throw 'Player display-list allocation anchor missing or duplicated'}
  $source=$source.Replace($spec[0],$spec[1])
 }
 if([regex]::Matches($source,[regex]::Escape($spec[1])).Count-ne 1){throw 'Player display-list replacement duplicated'}
}
$include='#include <Native/PlayerDisplayList.hpp>'
if(!$source.Contains($include)){$source=$include+"`n"+$source}
[IO.File]::WriteAllText($path,$source,[Text.UTF8Encoding]::new($false))