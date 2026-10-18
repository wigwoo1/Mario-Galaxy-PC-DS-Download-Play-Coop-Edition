$ErrorActionPreference='Continue'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\actor-runtime-probe";$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$source="$n\src\generated\J3DModelX.cpp";$unit='J3DModelX'
$header=Get-Content -Raw "$p\Petari\include\Game\Player\J3DModelX.hpp"
$fields=[regex]::Matches($header,'        unsigned _[0-9A-F]+ : 1;')
if($fields.Count -ne 32){throw 'Model flag field count changed'}
$reversed=@($fields | ForEach-Object {$_.Value});[array]::Reverse($reversed)
$first=$fields[0].Index;$last=$fields[31].Index+$fields[31].Length
$header=$header.Remove($first,$last-$first).Insert($first,($reversed -join "`n"))
$header=$header.Replace('*(u32*)this = 0;','*this = Flags{};')
New-Item -ItemType Directory -Force "$n\include-runtime\Game\Player" | Out-Null
Set-Content "$n\include-runtime\Game\Player\J3DModelX.hpp" $header
$original=Get-Content -Raw "$p\Petari\src\Game\Player\J3DModelX.cpp"
$initialization='    for(unsigned i=0;i<16;++i){mDisplayLists[i]=nullptr;mDisplayListSizes[i]=0;}' + "`n" + '    _1C4=nullptr;_1C8=nullptr;_1CC=nullptr;' + "`n" + '    struct ConstructorGuard{J3DModelX* model;bool complete=false;~ConstructorGuard(){if(complete)return;for(auto* list:model->mDisplayLists)if(list){Native::gpuResources().remove(list);JKRHeap::free(list,JKRHeap::findFromRoot(list));}delete[] model->_1C4;delete[] model->_1C8;delete[] model->_1CC;}} construction{this};' + "`n" + '    _DC = 0;'
$original=([regex]'    _DC = 0;').Replace($original,$initialization,1)
$original=$original.Replace('    mDisplayLists[index] = new (32) u8[size];','    if(index>=16)throw std::out_of_range("J3DModelX display-list index");' + "`n" + '    if(mDisplayLists[index]){Native::gpuResources().remove(mDisplayLists[index]);JKRHeap::free(mDisplayLists[index],JKRHeap::findFromRoot(mDisplayLists[index]));mDisplayLists[index]=nullptr;mDisplayListSizes[index]=0;}' + "`n" + '    mDisplayLists[index] = new (32) u8[size];')
$original=$original.Replace('    DCStoreRange(mDisplayLists[index], size);','    DCStoreRange(mDisplayLists[index], size);' + "`n" + '    Native::gpuResources().add(mDisplayLists[index],size);')
$original=$original.Replace('J3DModelX::~J3DModelX() {','J3DModelX::~J3DModelX() {' + "`n" + '    for(auto* list:mDisplayLists)if(list){Native::gpuResources().remove(list);JKRHeap::free(list,JKRHeap::findFromRoot(list));} delete[] _1C4;delete[] _1C8;delete[] _1CC;')
$original=$original.Replace('    Native::gpuResources().add(mDisplayLists[index],size);','    try{Native::gpuResources().add(mDisplayLists[index],size);}catch(...){JKRHeap::free(mDisplayLists[index],JKRHeap::findFromRoot(mDisplayLists[index]));mDisplayLists[index]=nullptr;throw;}')
$original=$original.Replace("    _DC = 0;`r`n}","    _DC = 0;`r`n    construction.complete=true;`r`n}")
if(!$original.Contains('construction.complete=true;')){throw 'Constructor completion anchor missing'}
$original=$original.Replace('    GDLObj obj;','    Native::GDCurrentScope writerScope;' + "`n" + '    GDLObj obj;')
$original=$original.Replace('    for (u32 i = 0; i <= 15; i++) {','    u32 nativeFlags;std::memcpy(&nativeFlags,&mFlags,sizeof(nativeFlags));' + "`n" + '    for (u32 i = 0; i <= 15; i++) {').Replace('*reinterpret_cast< u32* >(&mFlags)','nativeFlags')
$original='#include <cstring>'+"`n"+'#include <Native/GDCurrentScope.hpp>'+"`n"+'#include <Native/GpuResources.hpp>'+"`n"+'#include <stdexcept>'+"`n"+$original
$initCount=([regex]::Matches($original,'for\(unsigned i=0;i<16;\+\+i\)')).Count
if($initCount -ne 1 -or $original.IndexOf('mDisplayLists[i]=nullptr') -gt $original.IndexOf('storeDisplayList(&obj, 0);')){throw 'Effect list initialization order/count invalid'}
Set-Content $source ('#include <Native/HeapPlacement.hpp>'+"`n"+$original)
$line=Get-Content "$n\probe-actor-runtime.ps1" | Where-Object {$_ -like '& *zig.exe*'}
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\J3DModelX.log" -Tail 25;exit 1}
Write-Output 'Compiled original J3DModelX methods; runtime behavior not yet verified.'






