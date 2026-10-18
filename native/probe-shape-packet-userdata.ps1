$ErrorActionPreference='Continue'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\actor-runtime-probe";$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$unit='ShapePacketUserData';$source="$n\src\generated\ShapePacketUserData.cpp"
$header=Get-Content -Raw "$p\Petari\include\Game\System\ShapePacketUserData.hpp"
$header=$header.Replace('    ShapePacketUserData();','    ShapePacketUserData();'+"`n"+'    ~ShapePacketUserData();')
New-Item -ItemType Directory -Force "$n\include-runtime\Game\System" | Out-Null
Set-Content "$n\include-runtime\Game\System\ShapePacketUserData.hpp" $header
$original=Get-Content -Raw "$p\Petari\src\Game\System\ShapePacketUserData.cpp"
$original=$original.Replace('    MR::ProhibitSchedulerAndInterrupts prohibit(false);','    if(mDisplayList)throw std::logic_error("Shape packet userdata already initialized");'+"`n"+'    MR::ProhibitSchedulerAndInterrupts prohibit(false);')
$original=$original.Replace('    mTexGenNum = pMaterial->getTexGenNum();','    mTexGenNum = pMaterial->getTexGenNum();'+"`n"+'    if(mTexGenNum>8)throw std::out_of_range("Shape packet texture coordinate count");')
$original=$original.Replace('    mDisplayList = new (32) u8[size + 0x40];','    struct HeapDelete{void operator()(u8* bytes)const{if(bytes)JKRHeap::free(bytes,JKRHeap::findFromRoot(bytes));}};'+"`n"+'    std::unique_ptr<u8,HeapDelete> storage(new (32) u8[size+0x40]);mDisplayList=storage.get();'+"`n"+'    struct InitGuard{ShapePacketUserData* owner;bool complete=false;~InitGuard(){if(!complete){owner->mDisplayList=nullptr;owner->mDisplayListSize=0;}}} initialization{this};')
$original=$original.Replace('ShapePacketUserData* pUserData = new ShapePacketUserData();','std::unique_ptr<ShapePacketUserData> ownedUserData(new ShapePacketUserData()); ShapePacketUserData* pUserData=ownedUserData.get();')
$original=$original.Replace('pShapePacket->setUserArea(reinterpret_cast< uintptr_t >(pUserData));','pShapePacket->setUserArea(reinterpret_cast< uintptr_t >(pUserData)); ownedUserData.release();')
$original=$original.Replace('    GDLObj displayList;','    Native::GDCurrentScope writerScope;'+"`n"+'    GDLObj displayList;')
$original=$original.Replace('    DCStoreRange(mDisplayList, size + 0x40);','    DCStoreRange(mDisplayList, size + 0x40);'+"`n"+'    Native::gpuResources().add(mDisplayList,size+0x40);storage.release();initialization.complete=true;')
$original+="`nShapePacketUserData::~ShapePacketUserData(){if(mDisplayList){Native::gpuResources().remove(mDisplayList);JKRHeap::free(mDisplayList,JKRHeap::findFromRoot(mDisplayList));}}`n"
Set-Content $source ('#include <memory>'+"`n"+'#include <stdexcept>'+"`n"+'#include <Native/HeapPlacement.hpp>'+"`n"+'#include <Native/GDCurrentScope.hpp>'+"`n"+'#include <Native/GpuResources.hpp>'+"`n"+$original)
$line=Get-Content "$n\probe-actor-runtime.ps1" | Where-Object {$_ -like '& *zig.exe*'}
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 22;exit 1}


