param([Parameter(Mandatory)][string]$SourceText,[Parameter(Mandatory)][ValidateSet('MarioActorDraw','MarioActorSpecialDraw')][string]$Unit)
$ErrorActionPreference='Stop'
$methods=if($Unit-eq 'MarioActorDraw'){@('createTextureDL')}else{@('createRainbowDL','updateAlphaDL','updateSimpleAlphaDL','updateReflectAlphaDL','updateLightDL')}
foreach($method in $methods){
$pattern='void MarioActor::'+$method+'\([^\r\n]*\) \{'
$matchesFound=[regex]::Matches($SourceText,$pattern)
if($matchesFound.Count-ne 1){throw "Player private writer method anchor changed: $Unit"}
$anchor=$matchesFound[0].Value
$scoped=$anchor+"`n    Native::GDCurrentScope nativePrivateWriterScope;"
if(!$SourceText.Contains($scoped)){$SourceText=$SourceText.Replace($anchor,$scoped)}
}
if(!$SourceText.Contains('#include <Native/GDCurrentScope.hpp>')){$SourceText='#include <Native/GDCurrentScope.hpp>'+"`n"+$SourceText}
$SourceText