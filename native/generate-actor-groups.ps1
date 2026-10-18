$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\LiveActor\LiveActorGroupArray.cpp" -Raw
$b=$s.IndexOf('namespace {');$e=$s.IndexOf('MsgSharedGroup::MsgSharedGroup(')
if($b -lt 0 -or $e -lt $b){throw 'Missing original group predicates'}
$s=$s.Remove($b,$e-$b)
$b=$s.IndexOf('namespace {');$e=$s.IndexOf('LiveActorGroup* LiveActorGroupArray::findGroup(')
if($b -lt 0 -or $e -lt $b){throw 'Missing original group-id predicate'}
$s=$s.Remove($b,$e-$b)
$s=$s.Replace('std::find_if(mGroups.begin(), mGroups.end(), ::makeGroupIdPredicate(MR::createJMapIdInfoFromGroupId(rIter)))','std::find_if(mGroups.begin(), mGroups.end(), [id = MR::createJMapIdInfoFromGroupId(rIter)](const MsgSharedGroup* group){return *group->mIdInfo == id;})')
$s=$s.Replace('std::find_if(mGroups.begin(), mGroups.end(), std::binder2nd< HasActor, const LiveActor* >(HasActor(), pActor))','std::find_if(mGroups.begin(), mGroups.end(), [pActor](const MsgSharedGroup* group){return group->hasActor(pActor);})')
# Keep fallback name storage alive until createGroup copies it.
$s=$s.Replace('    if (pName == nullptr) {'+"`r`n"+'        char defaultName[32];','    char defaultName[32];'+"`r`n"+'    if (pName == nullptr) {')
if($s.Contains('std::binder2nd') -or $s.Contains('std::binary_function')){throw 'Obsolete group predicates remain'}
Set-Content "$n\src\generated\ActorGroupsKernel.cpp" $s
