$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
# Parse native matrix helpers before the legacy header adds its std::atan2 wrapper.
$prefix='#include <JSystem/JGeometry/TMatrix.hpp>'+[Environment]::NewLine+'#include <math_types.hpp>'+[Environment]::NewLine
$source=Get-Content "$p\Petari\src\Game\LiveActor\PartsModel.cpp" -Raw
Set-Content "$n\src\generated\PartsModelKernel.cpp" ($prefix+$source)
$source=Get-Content "$p\Petari\src\Game\Util\FixedPosition.cpp" -Raw
$source=$source.Replace('MR::getCsvDataVec(&trans, csv, "Trans", 0);','Vec nativeTrans{trans.x,trans.y,trans.z}; MR::getCsvDataVec(&nativeTrans, csv, "Trans", 0); trans.set(nativeTrans.x,nativeTrans.y,nativeTrans.z);')
$source=$source.Replace('MR::getCsvDataVec(&rotate, csv, "Rotate", 0);','Vec nativeRotate{rotate.x,rotate.y,rotate.z}; MR::getCsvDataVec(&nativeRotate, csv, "Rotate", 0); rotate.set(nativeRotate.x,nativeRotate.y,nativeRotate.z);')
Set-Content "$n\src\generated\FixedPositionKernel.cpp" ($prefix+$source)
