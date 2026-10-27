$ErrorActionPreference='Stop'
$n=$PSScriptRoot
foreach($spec in @(
 @('MarioActorDraw','new (std::align_val_t{32}) Mtx[0xC00 / sizeof(Mtx)]','Native::allocateExtraModelMatrices(0xC00 / sizeof(Mtx))'),
 @('MarioSearchLight','new (std::align_val_t{32}) Mtx[num]','Native::allocateExtraModelMatrices(num)')
)){
 $path=Join-Path $n ('src\generated\'+$spec[0]+'.cpp')
 $source=[IO.File]::ReadAllText($path)
 if(!$source.Contains($spec[2])){
  if([regex]::Matches($source,[regex]::Escape($spec[1])).Count-ne 1){throw ('Extra matrix allocation anchor missing or duplicated: '+$spec[0])}
  $source=$source.Replace($spec[1],$spec[2])
 }
 if([regex]::Matches($source,[regex]::Escape($spec[2])).Count-ne 1){throw ('Extra matrix replacement duplicated: '+$spec[0])}
 $include='#include <Native/ExtraModelMatrices.hpp>'
 if(!$source.Contains($include)){$source=$include+"`n"+$source}
 [IO.File]::WriteAllText($path,$source,[Text.UTF8Encoding]::new($false))
}