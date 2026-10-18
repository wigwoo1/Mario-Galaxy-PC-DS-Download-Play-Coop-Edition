$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\J3DGraphBase\J3DPacket.cpp'))
$s='#include <Native/HeapPlacement.hpp>'+"`n"+'#ifndef ALIGN_NEXT'+"`n"+'#define ALIGN_NEXT(X,N) (((X)+(N)-1)&~((N)-1))'+"`n"+'#endif'+"`n"+$s
$s=$s.Replace('GDLObj J3DDisplayListObj::sGDLObj;', 'thread_local GDLObj J3DDisplayListObj::sGDLObj;').Replace('s32 J3DDisplayListObj::sInterruptFlag;', 'thread_local s32 J3DDisplayListObj::sInterruptFlag;')
$anchor='void J3DDisplayListObj::beginDL() {'
if(!$s.Contains($anchor)){throw 'Packet begin writer anchor missing.'}
$s=$s.Replace($anchor,'namespace { thread_local GDLObj* nativePreviousPacketWriter=nullptr; }'+"`n"+$anchor+"`n"+'    nativePreviousPacketWriter=__GDCurrentDL;')
if(([regex]::Matches($s,'GDSetCurrent\(NULL\);')).Count-ne 2){throw 'Expected both packet end writer anchors.'}
$s=$s.Replace('GDSetCurrent(NULL);','GDSetCurrent(nativePreviousPacketWriter);'+"`n"+'    nativePreviousPacketWriter=nullptr;')
$anchor='J3DMatPacket::J3DMatPacket() {'
if(!$s.Contains($anchor)){throw 'Packet initialization anchor missing.'}
$s=$s.Replace($anchor,$anchor+"`n    mpShapePacket = NULL; // Native heap storage is not implicitly cleared.")
$s='#include <Native/HeapGpuRange.hpp>'+"`n"+$s
$double='    mpDisplayList[1] = new (0x20) char[mMaxSize];'+"`n"+'    mSize = 0;'
$single='    mpDisplayList[1] = mpDisplayList[0];'+"`n"+'    mSize = 0;'
$s=$s.Replace("`r`n","`n")
if(!$s.Contains($double)-or !$s.Contains($single)){throw 'Allocated list registration anchors changed.'}
$s=$s.Replace($double,$double+"`n"+'    Native::bindHeapGpuRange(mpDisplayList[0],mMaxSize);'+"`n"+'    Native::bindHeapGpuRange(mpDisplayList[1],mMaxSize);')
$s=$s.Replace($single,$single+"`n"+'    Native::bindHeapGpuRange(mpDisplayList[0],mMaxSize);')
$convert='        DCStoreRange(mpDisplayList[1], mMaxSize);'
if(!$s.Contains($convert)){throw 'Converted list registration anchor changed.'}
$s=$s.Replace($convert,$convert+"`n"+'        if(Native::hasHeapGpuRange(mpDisplayList[0]))Native::bindHeapGpuRange(mpDisplayList[1],mMaxSize);')
$s=$s.Replace('    GDFlushCurrToMem();','    GDFlushCurrToMem();'+"`n"+'    Native::gpuResources().refresh(mpDisplayList[0],mSize);')
$s=$s.Replace('u32 J3DDisplayListObj::endPatch() {','u32 J3DDisplayListObj::endPatch() {'+"`n"+'    Native::gpuResources().refresh(mpDisplayList[0],mSize);')
[IO.File]::WriteAllText((Join-Path $n 'src\generated\J3DPacket.cpp'),$s)