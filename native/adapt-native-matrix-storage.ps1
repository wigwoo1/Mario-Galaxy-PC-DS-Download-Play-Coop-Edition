param([Parameter(Mandatory)][string]$SourceText)
$ErrorActionPreference='Stop'
$edits=@(
 @('mpDrawMtxArr[i][j] = new (0x20) Mtx[pModelData->getDrawMtxNum()];','Native::bindHeapGpuNativeWords(mpDrawMtxArr[i][j],std::size_t(pModelData->getDrawMtxNum())*sizeof(Mtx));'),
 @('mpNrmMtxArr[i][j] = new (0x20) Mtx33[pModelData->getDrawMtxNum()];','Native::bindHeapGpuNativeWords(mpNrmMtxArr[i][j],std::size_t(pModelData->getDrawMtxNum())*sizeof(Mtx33));'),
 @('mpBumpMtxArr[i][offset][k] = new (0x20) Mtx33[i_modelData->getDrawMtxNum()];','if(i_modelData->getDrawMtxNum())Native::bindHeapGpuNativeWords(mpBumpMtxArr[i][offset][k],std::size_t(i_modelData->getDrawMtxNum())*sizeof(Mtx33));')
)
foreach($edit in $edits){
 if(([regex]::Matches($SourceText,[regex]::Escape($edit[0]))).Count-ne 1){throw 'Matrix allocation registration anchor changed.'}
 if(!$SourceText.Contains($edit[1])){$SourceText=$SourceText.Replace($edit[0],$edit[0]+"`n                "+$edit[1])}
}
if(!$SourceText.Contains('#include <Native/HeapGpuRange.hpp>')){$SourceText='#include <Native/HeapGpuRange.hpp>'+"`n"+$SourceText}
$SourceText