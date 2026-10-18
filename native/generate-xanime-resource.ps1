$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Animation\XanimeResource.cpp" -Raw
$a='case 1:'+"`r`n"+'        s32 groupIndex = getGroupIndex(pArg);'
$b='return &mGroupInfos[groupIndex];'
if(!$s.Contains($a) -or !$s.Contains($b)){throw 'Animation resource case anchors changed'}
$s=$s.Replace($a,'case 1: {'+"`r`n"+'        s32 groupIndex = getGroupIndex(pArg);').Replace($b,$b+"`r`n"+'    }')
Set-Content "$n\src\generated\XanimeResource.cpp" $s -Encoding UTF8
