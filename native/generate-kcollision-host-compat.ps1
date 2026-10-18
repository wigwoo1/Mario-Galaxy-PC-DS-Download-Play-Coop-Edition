$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Get-Content -LiteralPath "$p\Petari\src\Game\Map\KCollision.cpp" -Raw
function ExtractOriginalDefinition([string]$text,[string]$signature){
    $start=$text.IndexOf($signature,[StringComparison]::Ordinal)
    if($start -lt 0){throw "Missing original collision method: $signature"}
    $open=$text.IndexOf('{',$start)
    if($open -lt 0){throw "Missing body for original collision method: $signature"}
    $depth=0
    for($i=$open;$i -lt $text.Length;$i++){
        if($text[$i] -eq '{'){$depth++}
        elseif($text[$i] -eq '}'){
            $depth--
            if($depth -eq 0){return $text.Substring($start,$i-$start+1)}
        }
    }
    throw "Unclosed body for original collision method: $signature"
}
function ReplaceOnce([string]$text,[string]$old,[string]$new,[string]$label){
    $count=([regex]::Matches($text,[regex]::Escape($old))).Count
    if($count -ne 1){throw "Expected one $label source anchor, found $count"}
    return $text.Replace($old,$new)
}
$methods=@(
    (ExtractOriginalDefinition $source 'void KCollisionServer::init('),
    (ExtractOriginalDefinition $source 'void KCollisionServer::setData('),
    (ExtractOriginalDefinition $source 'bool KCollisionServer::isBinaryInitialized('),
    (ExtractOriginalDefinition $source 'KC_PrismData* KCollisionServer::checkPoint('),
    (ExtractOriginalDefinition $source 'u32 KCollisionServer::checkArea3D('),
    (ExtractOriginalDefinition $source 'u32 KCollisionServer::checkSphereWithThickness('),
    (ExtractOriginalDefinition $source 'bool KCollisionServer::KCHitSphereWithThickness('),
    (ExtractOriginalDefinition $source 'Fxyz& Fxyz::operator=(')
)
$methods[6]=ReplaceOnce $methods[6] 'bool KCollisionServer::KCHitSphereWithThickness(KC_PrismData* pPrism, Fxyz* pCenter, f32 radius, f32 param, f32* pDist, u8* pFlag, f32 thickness) {' ("bool KCollisionServer::KCHitSphereWithThickness(KC_PrismData* pPrism, Fxyz* pCenter, f32 radius, f32 param, f32* pDist, u8* pFlag, f32 thickness) {`n    f32 distance;") 'collision sphere local declaration'
$methods[6]=ReplaceOnce $methods[6] '    f32 distance = MR::sqrt(*pDist);' '    distance = MR::sqrt(*pDist);' 'collision sphere distance calculation'
$header=@'
#include "Game/Map/KCollision.hpp"
#include "Game/Camera/CameraPolygonCodeUtil.hpp"
#include "Game/Map/CollisionCode.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/MathUtil.hpp"
#include <algorithm>
'@
Set-Content -LiteralPath "$n\src\generated\KCollisionQueriesHost.cpp" -Value ($header+"`n"+($methods -join "`n`n")) -NoNewline
