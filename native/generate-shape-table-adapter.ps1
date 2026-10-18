$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\libs\JSystem\include\JSystem\J3DGraphAnimator\J3DShapeTable.hpp" -Raw
if(!$source.Contains('friend class J3DModelLoader;')){throw 'Shape table adapter anchor missing'}
$source=$source.Replace('friend class J3DModelLoader;','friend class J3DModelLoader;'+"`r`n"+'    friend struct NativeModelResource;')
New-Item -ItemType Directory -Path "$n\include\JSystem\J3DGraphAnimator" -Force | Out-Null
Set-Content "$n\include\JSystem\J3DGraphAnimator\J3DShapeTable.hpp" $source
