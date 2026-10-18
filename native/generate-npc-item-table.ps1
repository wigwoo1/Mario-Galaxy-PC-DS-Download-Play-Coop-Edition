$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\Game\NPC\NPCFunction.cpp'))
$anchor='    mapInfo.attach(pHolder->mFileInfoTable->getRes(resName));'
if([regex]::Matches($source,[regex]::Escape($anchor)).Count-ne 1){throw 'NPC item table attachment anchor missing or duplicated'}
$replacement=@'
    ResFileInfo* resource = pHolder->mFileInfoTable->findFileInfo(resName);
    if (!resource) throw std::runtime_error("NPC item resource disappeared during lookup");
    mapInfo.attach(Native::npcItemTable(*resource, pHolder->mHeap));
'@
$source='#include <Native/NpcItemTable.hpp>'+"`n"+$source.Replace($anchor,$replacement)
[IO.File]::WriteAllText((Join-Path $n 'src\generated\NPCFunction.cpp'),$source,[Text.UTF8Encoding]::new($false))

$holderPath=Join-Path $n 'src\generated\ResourceHolder.cpp'
$holder=[IO.File]::ReadAllText($holderPath)
$prepare='            Native::prepareNpcItemTable(pArchive->mLoaderName, *info, mHeap);'
if(!$holder.Contains($prepare)){
 $metadata='            info->_C = finder->mFileID;'
 if([regex]::Matches($holder,[regex]::Escape($metadata)).Count-ne 1){throw 'NPC archive registration metadata anchor missing or duplicated'}
 $holder='#include <Native/NpcItemTable.hpp>'+"`n"+$holder.Replace($metadata,$metadata+"`n"+$prepare)
 [IO.File]::WriteAllText($holderPath,$holder,[Text.UTF8Encoding]::new($false))
}
