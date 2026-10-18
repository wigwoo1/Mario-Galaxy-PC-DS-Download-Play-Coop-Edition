$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content -Raw "$p\Petari\src\Game\Util\ModelUtil.cpp"
function Extract-Function([string]$name){
 $start=$source.IndexOf('    bool '+$name+'(');if($start -lt 0){$start=$source.IndexOf('    void '+$name+'(')}
 if($start -lt 0){throw "Missing original function $name"}
 $brace=$source.IndexOf('{',$start);$depth=1;$end=$brace+1
 while($depth -gt 0){if($source[$end] -eq '{'){$depth++}elseif($source[$end] -eq '}'){$depth--};$end++}
 return 'inline '+$source.Substring($start,$end-$start).Trim()
}
$header="#pragma once`n#include `"native_model_resource.hpp`"`nnamespace NativeModelInit {`n"
foreach($name in @('isEnvelope','isUseTexMtx','isUseTexMtxEnvMap','isUseTexMtxProjMap')){$header+=(Extract-Function $name)+"`n"}
$init=Extract-Function 'initEnvelopeAndEnvMapOrProjMapModelData'
$init=$init.Replace('J3DModelData* pModelData)','J3DModelData* pModelData, NativeModelResource& resources, JKRHeap& heap)')
$init=$init.Replace('setShapeVcdVatCmdSelf(shape);','resources.shapes->makeCommandPrivate(*shape, heap);')
$init=$init.Replace('shape->addTexMtxIndexInVcd(attr);','shape->addTexMtxIndexInVcd(attr); resources.shapes->retainRewrittenGeometry(); resources.shapes->retainRewrittenDescriptors();')
$init=$init.Replace('        if (doSort) {','        resources.shapes->prepareVertexCommands(pModelData->getVertexData());' + "`n" + '        if (doSort) {')
$header+=$init+"`n}`n"
Set-Content "$n\src\native_model_env_init.hpp" $header
Set-Content "$n\src\generated\ModelEffectChecks.cpp" @"
#include "../native_model_env_init.hpp"
namespace MR {
bool isEnvelope(J3DMaterial* material){return NativeModelInit::isEnvelope(material);}
bool isUseTexMtxEnvMap(J3DMaterial* material){return NativeModelInit::isUseTexMtxEnvMap(material);}
bool isUseTexMtxProjMap(J3DMaterial* material){return NativeModelInit::isUseTexMtxProjMap(material);}
}
"@
