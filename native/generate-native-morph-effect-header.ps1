$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\include\Game\MapObj\MorphItemObjNeo.hpp" -Raw
foreach($type in @('EffectObj20x20x10SyncClipping','EffectObj50x50x10SyncClipping','EffectObj10x10x10SyncClipping')){$old="class $type : public SimpleEffectObj {";if(!$s.Contains($old)){throw "Missing $type"};$s=$s.Replace($old,$old+"`r`n    mutable TVec3f mNativeClippingOffset;")}
$s=$s.Replace('return &(TVec3f(0.0f, 200.0f, 0.0f));','mNativeClippingOffset.set(0.0f, 200.0f, 0.0f); return &mNativeClippingOffset;')
$s=$s.Replace('return &TVec3f(0.0f, 580.0f, 0.0f);','mNativeClippingOffset.set(0.0f, 580.0f, 0.0f); return &mNativeClippingOffset;')
New-Item -ItemType Directory -Force "$n\include-runtime\Game\MapObj"|Out-Null
$s|Set-Content "$n\include-runtime\Game\MapObj\MorphItemObjNeo.hpp"
