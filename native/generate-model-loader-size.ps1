$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\JSystem\J3DGraphLoader\J3DModelLoaderCalcSize.cpp" -Raw
$replacements=@{
    'size += 0xE4;'='size += sizeof(J3DModelData);'
    'size = 0x20;'='size = sizeof(J3DMaterialTable);'
    'size += 12;'='size += sizeof(J3DTexture);'
    'size = 0x10;'='size = sizeof(JUTNameTab);'
    'padding = 0x10;'='padding = sizeof(JUTNameTab);'
    'return padding + 0xC;'='return padding + sizeof(J3DTexture);'
    'size = 4;'='size = sizeof(J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformBasic,J3DMtxCalcJ3DSysInitBasic>);'
}
foreach($entry in $replacements.GetEnumerator()){
    if(!$source.Contains($entry.Key)){throw "Missing original loader size anchor: $($entry.Key)"}
    $source=$source.Replace($entry.Key,$entry.Value)
}
# Each matrix calculator has its own native type; all original branches were 4 bytes.
$source=$source.Replace('case J3DMLF_MtxSoftImageCalc:' + "`r`n" + '        size = sizeof(J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformBasic,J3DMtxCalcJ3DSysInitBasic>);',
 'case J3DMLF_MtxSoftImageCalc:' + "`r`n" + '        size = sizeof(J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformSoftimage,J3DMtxCalcJ3DSysInitSoftimage>);')
$source=$source.Replace('case J3DMLF_MtxMayaCalc:' + "`r`n" + '        size = sizeof(J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformBasic,J3DMtxCalcJ3DSysInitBasic>);',
 'case J3DMLF_MtxMayaCalc:' + "`r`n" + '        size = sizeof(J3DMtxCalcNoAnm<J3DMtxCalcCalcTransformMaya,J3DMtxCalcJ3DSysInitMaya>);')
Set-Content "$n\src\generated\J3DModelLoaderCalcSize.cpp" -Encoding UTF8 -Value ('// Native object-size adaptation; console data still requires host conversion.'+"`r`n"+'#include <JSystem/JUtility/JUTNameTab.hpp>'+"`r`n"+$source)
