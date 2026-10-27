$ErrorActionPreference='Stop'
$path=Join-Path $PSScriptRoot 'src\generated\ResourceHolder.cpp'
$source=[IO.File]::ReadAllText($path)
$original='return mBrkResTable->add(pName, J3DAnmLoaderDataBase::load(pData), true);'
$replacement='return mBrkResTable->add(pName, loadNativeArchiveTevReg(*mArchive, pData, mHeap), true);'
if(!$source.Contains($replacement)){
 if([regex]::Matches($source,[regex]::Escape($original)).Count-ne 1){throw 'Original BRK attachment anchor missing or duplicated'}
 $source='#include "../native_owned_tev_reg.hpp"'+"`n"+$source.Replace($original,$replacement)
 [IO.File]::WriteAllText($path,$source,[Text.UTF8Encoding]::new($false))
}
