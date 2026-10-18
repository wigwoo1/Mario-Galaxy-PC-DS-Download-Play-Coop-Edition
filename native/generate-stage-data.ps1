$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=[IO.File]::ReadAllText("$p\Petari\include\Game\Scene\StageDataHolder.hpp")
foreach($field in @('_E4','_E8')){if(!$h.Contains("u32 $field;")){throw "Address field anchor: $field"};$h=$h.Replace("u32 $field;","std::uintptr_t $field;")}
$h=$h.Replace('#pragma once',"#pragma once`n#include <cstdint>")
$h=$h.Replace('#include <cstdint>',"#include <cstdint>`n#include <Native/StageArchiveTables.hpp>")
$h=$h.Replace('StageDataHolder(const char* pStageName, int zoneID, bool loadCommon);',"StageDataHolder(const char* pStageName, int zoneID, bool loadCommon, NativeStageArchiveTables* nativeTables = nullptr);`n    NativeStageArchiveTables* mNativeArchiveTables = nullptr;`n    void bindNativeArchiveTables(NativeStageArchiveTables* tables) { mNativeArchiveTables = tables; }`n    s32 countStageFiles(const char* directory);")
[IO.File]::WriteAllText("$n\include-runtime\Game\Scene\StageDataHolder.hpp",$h)
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\Scene\StageDataHolder.cpp")
$stageAttachHelper=@'
    static void attachStageArchiveTable(JMapInfo& info, JKRArchive* archive, const void* resource) {
        if (!archive || !resource) throw std::runtime_error("Missing stage JMap resource");
        const s32 bytes = archive->getResSize(resource);
        if (bytes < 16) throw std::runtime_error("Invalid stage JMap resource extent");
        const auto* first = static_cast<const std::uint8_t*>(resource);
        info.attachNative(std::vector<std::uint8_t>(first, first + bytes));
    }
'@
$namespaceAt=$s.IndexOf('namespace {')
if($namespaceAt-lt 0){throw 'Stage table normalization helper anchor'}
$s=$s.Insert($namespaceAt+'namespace {'.Length,[Environment]::NewLine+$stageAttachHelper+[Environment]::NewLine)
$rawStageAttach='pInfo->attach(mArchive->getIdxResource(pFinder->mDirIndex));'
if(!$s.Contains($rawStageAttach)){throw 'Raw stage attachment anchor'}
$s=$s.Replace($rawStageAttach,'attachStageArchiveTable(*pInfo, mArchive, mArchive->getIdxResource(pFinder->mDirIndex));')
$rawNames='void* tableFile = MR::receiveArchive("/StageData/ObjNameTable.arc")->getResource(''????'', "ObjNameTable.tbl");'
if(!$s.Contains($rawNames)){throw 'Object-name archive attachment anchor'}
$s=$s.Replace($rawNames,'JKRArchive* tableArchive = MR::receiveArchive("/StageData/ObjNameTable.arc");'+[Environment]::NewLine+'    if (!tableArchive) throw std::runtime_error("Missing stage object-name archive");'+[Environment]::NewLine+'    void* tableFile = tableArchive->getResource(''????'', "ObjNameTable.tbl");')
$s=$s.Replace('    mObjNameTbl = new JMapInfo();','    auto table = std::make_unique<JMapInfo>();').Replace('    mObjNameTbl->attach(tableFile);','    attachStageArchiveTable(*table, tableArchive, tableFile);'+[Environment]::NewLine+'    mObjNameTbl = table.release();')
$s="#include <memory>`n#include <vector>`n#include <stdexcept>`n"+$s
foreach($anchor in @('s32 data = (s32)rIter.mInfo->mData','(u32)pInfo->mData','u32 addr =','((s32)pInfo->mData +')){if(!$s.Contains($anchor)){throw "Stage pointer anchor missing: $anchor"}}
$s=$s.Replace('s32 data = (s32)rIter.mInfo->mData','std::uintptr_t data = reinterpret_cast<std::uintptr_t>(rIter.mInfo->mData)')
$s=$s.Replace('(u32)pInfo->mData','reinterpret_cast<std::uintptr_t>(pInfo->mData)')
$s=$s.Replace('u32 addr =','std::uintptr_t addr =').Replace('((s32)pInfo->mData +','(reinterpret_cast<std::uintptr_t>(pInfo->mData) +')
$ctor='StageDataHolder::StageDataHolder(const char* pStageName, int zoneID, bool loadCommon)'
if(!$s.Contains($ctor) -or !$s.Contains('mArchive(MR::getStageArchive(pStageName))')){throw 'Native stage constructor anchor missing'}
$s=$s.Replace($ctor,'StageDataHolder::StageDataHolder(const char* pStageName, int zoneID, bool loadCommon, NativeStageArchiveTables* nativeTables)')
$s=$s.Replace('mArchive(MR::getStageArchive(pStageName))','mArchive(nativeTables ? nullptr : MR::getStageArchive(pStageName))')
$s=$s.Replace('    MR::zeroMemory(mStageDataArray, sizeof(mStageDataArray));',"    mNativeArchiveTables = nativeTables;`n    MR::zeroMemory(mStageDataArray, sizeof(mStageDataArray));")
$anchor='JMapInfo* StageDataHolder::attachJmpInfoToArray(JMapInfo* pInfo, const char* pDirectory) {'
if(!$s.Contains($anchor)){throw 'Stage attachment hook anchor missing'}
$s=$s.Replace('mArchive->countFile(', 'countStageFiles(')
$s=$s.Replace($anchor,$anchor+"`n    if (mNativeArchiveTables) return mNativeArchiveTables->appendTables(pInfo, pDirectory);")
$s=$s.Replace("return mArchive->getResource('????', pName);","return mNativeArchiveTables ? mNativeArchiveTables->resource(pName) : mArchive->getResource('????', pName);")
$s=$s.Replace('return mArchive->getResSize(pData);','return mNativeArchiveTables ? mNativeArchiveTables->resourceSize(pData) : mArchive->getResSize(pData);')
$child='new StageDataHolder(pStageName, zoneID, loadCommon);'
if(!$s.Contains($child)){throw 'Child stage constructor anchor missing'}
$s=$s.Replace($child,'new StageDataHolder(pStageName, zoneID, loadCommon, mNativeArchiveTables ? mNativeArchiveTables->resolveStage(pStageName) : nullptr);')
$objectInit='void StageDataHolder::initTableData() {'
if(!$s.Contains($objectInit)){throw 'Object-name table init anchor missing'}
$s=$s.Replace($objectInit,$objectInit+"`n    if (mNativeArchiveTables) { mObjNameTbl = new JMapInfo(*mNativeArchiveTables->objectNameTable()); return; }")
$ownership='if (_E4 <= data && data < _E8) {'
if(!$s.Contains($ownership)){throw 'Stage ownership hook missing'}
$s=$s.Replace($ownership,'if (nativeStageArrayOwns(mPlacementObjs, rIter) || nativeStageArrayOwns(mStartObjs, rIter) || nativeStageArrayOwns(mGeneralPosObjs, rIter) || nativeStageArrayOwns(mChildObjs, rIter) || nativeStageArrayOwns(mPathObjs, rIter) || nativeStageArrayOwns(_EC, rIter) || nativeStageArrayOwns(_F4, rIter)) {')
$stageDataAddressPattern='(?m)^    std::uintptr_t data = reinterpret_cast<std::uintptr_t>\(rIter\.mInfo->mData\)[^\r\n]*;'
if([regex]::Matches($s,$stageDataAddressPattern).Count-ne 1){throw 'Stage iterator validation anchor'}
$s=[regex]::Replace($s,$stageDataAddressPattern,'    if (!rIter.mInfo || !rIter.mInfo->mData || rIter.mIndex < 0 || rIter.mIndex >= rIter.mInfo->getNumEntries()) return nullptr;')
$s += "`ns32 StageDataHolder::countStageFiles(const char* directory) {`n    return mNativeArchiveTables ? mNativeArchiveTables->countTables(directory) + 2 : mArchive->countFile(directory);`n}`n"
[IO.File]::WriteAllText("$n\src\generated\StageDataHolder.cpp",$s)
New-Item -ItemType Directory -Force "$n\include-runtime\Game\Util" | Out-Null
Copy-Item -LiteralPath "$p\Petari\include\Game\Util\StringUtil.hpp" -Destination "$n\include-runtime\Game\Util\StringUtil.hpp"







New-Item -ItemType Directory -Force "$n\include-runtime\Game\System" | Out-Null
$scenarioHeader=[IO.File]::ReadAllText("$p\Petari\include\Game\System\ScenarioDataParser.hpp")
$scenarioHeader=$scenarioHeader.Replace('#pragma once',"#pragma once`n#include <Game/Util/JMapInfo.hpp>`n#include <memory>`n#include <cstring>")
$scenarioHeader=$scenarioHeader.Replace('    ScenarioData(const char*);','    ScenarioData(const char*); std::shared_ptr<JMapInfo> mNativeScenarioOwner, mNativeZoneOwner; std::shared_ptr<char[]> mNativeNameOwner; ScenarioData(const char* name,const JMapInfo& scenarios,const JMapInfo& zones) : mScenarioData(nullptr),mGalaxyName(nullptr),mZoneList(nullptr),mNativeScenarioOwner(std::make_shared<JMapInfo>(scenarios)),mNativeZoneOwner(std::make_shared<JMapInfo>(zones)) {size_t length=std::strlen(name)+1;mNativeNameOwner=std::shared_ptr<char[]>(new char[length]);std::memcpy(mNativeNameOwner.get(),name,length);mGalaxyName=mNativeNameOwner.get();mScenarioData=mNativeScenarioOwner.get();mZoneList=mNativeZoneOwner.get();}')
[IO.File]::WriteAllText("$n\include-runtime\Game\System\ScenarioDataParser.hpp",$scenarioHeader)
$scenarioSource=[IO.File]::ReadAllText("$p\Petari\src\Game\System\ScenarioDataParser.cpp")
$scenarioAttachHelper=@'
static void attachScenarioTable(JMapInfo& info, JKRArchive* archive, const char* name) {
    if (!archive) throw std::runtime_error("Scenario archive is missing");
    const void* resource = archive->getResource(name);
    if (!resource) throw std::runtime_error(std::string("Scenario table is missing: ") + name);
    const s32 bytes = archive->getResSize(resource);
    if (bytes <= 0) throw std::runtime_error("Scenario table has no valid archive extent");
    const auto* first = static_cast<const std::uint8_t*>(resource);
    info.attachNative(std::vector<std::uint8_t>(first, first + bytes));
}

'@
$scenarioCtorAnchor='ScenarioData::ScenarioData(const char* pFilePath)'
if(!$scenarioSource.Contains($scenarioCtorAnchor)){throw 'Scenario resource constructor anchor missing'}
$scenarioSource=$scenarioSource.Replace($scenarioCtorAnchor,$scenarioAttachHelper + "`n" + $scenarioCtorAnchor)
foreach($table in @(@('mScenarioData','/ScenarioData.bcsv'),@('mZoneList','/ZoneList.bcsv'))){
 $attachment=$table[0]+'->attach(pArchive->getResource("'+$table[1]+'"));'
 if(!$scenarioSource.Contains($attachment)){throw "Scenario table attachment anchor missing: $($table[1])"}
 $scenarioSource=$scenarioSource.Replace($attachment,'attachScenarioTable(*'+$table[0]+', pArchive, "'+$table[1]+'");')
}
$kernelStart=$scenarioSource.IndexOf('s32 ScenarioData::getScenarioNum() const')
$kernelEnd=$scenarioSource.IndexOf('ScenarioDataIter::ScenarioDataIter(')
if($kernelStart -lt 0 -or $kernelEnd -le $kernelStart){throw 'Scenario method kernel anchor missing'}
$kernel="#include <Game/System/ScenarioDataParser.hpp>`n#include <Game/Util/StringUtil.hpp>`n"+$scenarioSource.Substring($kernelStart,$kernelEnd-$kernelStart)
[IO.File]::WriteAllText("$n\src\generated\ScenarioDataKernel.cpp",$kernel)

$scenarioHeader=$scenarioHeader.Replace('#pragma once',"#pragma once`n#include <Native/ScenarioSource.hpp>")
$scenarioHeader=$scenarioHeader.Replace('ScenarioDataParser(const char* pName);','ScenarioDataParser(const char* pName); ScenarioDataParser(const char* pName, NativeScenarioSource* nativeSource);')
[IO.File]::WriteAllText("$n\include-runtime\Game\System\ScenarioDataParser.hpp",$scenarioHeader)
$parserAnchor='ScenarioDataParser::ScenarioDataParser(const char* pName) : NameObj(pName), mScenarioData() {'
if(!$scenarioSource.Contains($parserAnchor)){throw 'Native parser constructor anchor missing'}
$parserReplacement='ScenarioDataParser::ScenarioDataParser(const char* pName, NativeScenarioSource* nativeSource) : NameObj(pName), mScenarioData() { if (!nativeSource) nativeSource = nativeScenarioStartupSource; if (nativeSource) { auto source = nativeSource->scenarioList(); if (source.size() > 64) throw std::runtime_error("Native scenario list exceeds original capacity"); std::vector<std::unique_ptr<ScenarioData>> owned; for (const ScenarioData* data : source) { if (!data) throw std::runtime_error("Null native scenario source entry"); owned.emplace_back(new ScenarioData(*data)); } for (auto& data : owned) mScenarioData.mArray[mScenarioData.mCount++] = data.release(); std::sort(mScenarioData.begin(), mScenarioData.end(), GalaxyNameSortLt()); return; }'
$scenarioSource=$scenarioSource.Replace($parserAnchor,$parserReplacement)
$scenarioSource += "`nScenarioDataParser::ScenarioDataParser(const char* pName) : ScenarioDataParser(pName, nullptr) {}`n"
$scenarioSource=$scenarioSource.Replace('    DVDDir dir;', "#if defined(GALAXY_NATIVE_SCENARIO_SOURCE_ONLY)`n" + '    throw std::runtime_error("Native scenario source required");' + "`n#else`n    DVDDir dir;")
$scenarioSource=$scenarioSource.Replace("    std::sort(mScenarioData.begin(), mScenarioData.end(), GalaxyNameSortLt());`n}","    std::sort(mScenarioData.begin(), mScenarioData.end(), GalaxyNameSortLt());`n#endif`n}")
$scenarioSource=$scenarioSource.Replace("    std::sort(mScenarioData.begin(), mScenarioData.end(), GalaxyNameSortLt());`r`n}","    std::sort(mScenarioData.begin(), mScenarioData.end(), GalaxyNameSortLt());`r`n#endif`r`n}")
[IO.File]::WriteAllText("$n\src\generated\ScenarioDataParser.cpp","#include <stdexcept>`n"+$scenarioSource)
$galaxySource=[IO.File]::ReadAllText("$p\Petari\src\Game\System\GalaxyNameSortTable.cpp")
$galaxyBytes=[IO.File]::ReadAllBytes("$p\Petari\src\Game\System\GalaxyID.bcsv")
$galaxyArray='static const std::vector<std::uint8_t> nativeGalaxyID = {'+($galaxyBytes -join ',')+'};'
if(!$galaxySource.Contains('info.attach(&GalaxyIDBCSV);')){throw 'Galaxy sort table attachment anchor missing'}
$galaxySource=$galaxySource.Replace('extern const JMapData GalaxyIDBCSV;',$galaxyArray).Replace('info.attach(&GalaxyIDBCSV);','if (!info.attachNative(nativeGalaxyID)) throw std::runtime_error("Invalid embedded galaxy ID table");')
[IO.File]::WriteAllText("$n\src\generated\GalaxyNameSortTable.cpp","#include <stdexcept>`n"+$galaxySource)

$startupIncludes="#include <Game/System/ScenarioDataParser.hpp>`n#include <Game/System/GalaxyNameSortTable.hpp>`n#include <Game/Util/StringUtil.hpp>`n#include <algorithm>`n#include <stdexcept>`n"
$comparatorStart=$scenarioSource.IndexOf('namespace {')
$comparatorEnd=$scenarioSource.IndexOf('    const ScenarioData* getCurrentScenarioData()')
$ctorStart=$scenarioSource.IndexOf('ScenarioDataParser::ScenarioDataParser(')
$ctorEnd=$scenarioSource.IndexOf('#pragma pop',$ctorStart)
$lookupStart=$scenarioSource.IndexOf('const ScenarioData* ScenarioDataParser::getScenarioData(')
$lookupEnd=$scenarioSource.IndexOf('GalaxyStatusAccessor ScenarioDataParser::makeAccessor(')
if($comparatorStart -lt 0 -or $ctorEnd -lt 0 -or $lookupEnd -lt 0){throw 'Parser startup kernel anchors missing'}
$startup=$startupIncludes+$scenarioSource.Substring($comparatorStart,$comparatorEnd-$comparatorStart)+"}`n"+$scenarioSource.Substring($ctorStart,$ctorEnd-$ctorStart)+$scenarioSource.Substring($lookupStart,$lookupEnd-$lookupStart)+"`nScenarioDataParser::ScenarioDataParser(const char* pName) : ScenarioDataParser(pName, nullptr) {}`n"
[IO.File]::WriteAllText("$n\src\generated\ScenarioParserStartupKernel.cpp",$startup)
$scenarioSource="#include <Native/ScenarioBinding.hpp>`n"+$scenarioSource
$scenarioSource=$scenarioSource.Replace('getScenarioData(MR::getCurrentStageName())','getScenarioData(nativeScenarioBinding ? nativeScenarioBinding->galaxyName.c_str() : MR::getCurrentStageName())')
$serviceAnchor='        return SingletonHolder< GameSystem >::get()->mSceneController->mScenarioParser;'
if(!$scenarioSource.Contains($serviceAnchor)){throw 'Native scenario service anchor missing'}
$serviceReplacement="        if (nativeScenarioBinding) return nativeScenarioBinding->parser;`n#if defined(GALAXY_NATIVE_SCENARIO_SOURCE_ONLY)`n"+'        throw std::runtime_error("Native scenario services are not bound");'+"`n#else`n"+$serviceAnchor+"`n#endif"
$scenarioSource=$scenarioSource.Replace($serviceAnchor,$serviceReplacement)
[IO.File]::WriteAllText("$n\src\generated\ScenarioDataParser.cpp","#include <stdexcept>`n"+$scenarioSource)
$serviceStart=$scenarioSource.IndexOf('namespace ScenarioDataFunction {')
$serviceEnd=$scenarioSource.IndexOf('namespace MR {',$serviceStart)
$helper='namespace { const ScenarioData* getCurrentScenarioData() { if (!nativeScenarioBinding) throw std::runtime_error("Native scenario services are not bound"); return ScenarioDataFunction::getScenarioDataParser()->getScenarioData(nativeScenarioBinding->galaxyName.c_str()); } }'
[IO.File]::WriteAllText("$n\src\generated\ScenarioServicesKernel.cpp","#include <Native/ScenarioBinding.hpp>`n"+$helper+"`n"+$scenarioSource.Substring($serviceStart,$serviceEnd-$serviceStart))


