$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\StageSwitch.cpp" -Raw
$b=$s.IndexOf('StageSwitchContainer::~StageSwitchContainer()');if($b -lt 0){throw 'Missing original switch destructor'}
$o=$s.IndexOf('{',$b);$e=$s.IndexOf('}',$o)+1
$replacement='StageSwitchContainer::~StageSwitchContainer() { for (int i=0;i<mSwitches.size();++i) delete mSwitches[i].mSwitch; delete mGlobalSwitches; }'
$s=$s.Substring(0,$b)+$replacement+$s.Substring($e)
Set-Content "$n\src\generated\StageSwitchLifetimeKernel.cpp" $s
$parts=@($s.Substring(0,$s.IndexOf('SwitchIdInfo::SwitchIdInfo')))
foreach($signature in @('StageSwitchContainer::StageSwitchContainer()','StageSwitchContainer::~StageSwitchContainer()','SwitchIdInfo::SwitchIdInfo(','s32 SwitchIdInfo::getSwitchNo()','void StageSwitchContainer::createAndAddZone(','ZoneSwitch* StageSwitchContainer::getZoneSwitch(','ZoneSwitch* StageSwitchContainer::findZoneSwitchFromTable(')){
 $b=$s.IndexOf($signature);$o=$s.IndexOf('{',$b);$d=1;$e=$o+1
 while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++}
 $parts+=$s.Substring($b,$e-$b)
}
$id=Get-Content "$p\Petari\src\Game\Util\JMapIdInfo.cpp" -Raw
$parts+=$id.Substring(0,$id.IndexOf('namespace MR {'))
Set-Content "$n\src\generated\StageSwitchLifetimeFixture.cpp" ($parts -join [Environment]::NewLine)
$routing=$parts
$scene=Get-Content "$p\Petari\src\Game\Scene\SceneObjHolder.cpp" -Raw
$name=Get-Content "$p\Petari\src\Game\NameObj\NameObj.cpp" -Raw
$routing+=$name.Substring($name.IndexOf('void NameObj::initWithoutIter()'),$name.IndexOf('void NameObj::setName(')-$name.IndexOf('void NameObj::initWithoutIter()'))
foreach($spec in @(@($s,'SwitchIdInfo* StageSwitchFunction::createSwitchIdInfo('),@($s,'void StageSwitchFunction::onSwitchBySwitchIdInfo('),@($s,'void StageSwitchFunction::offSwitchBySwitchIdInfo('),@($s,'bool StageSwitchFunction::isOnSwitchBySwitchIdInfo('),@($scene,'SceneObjHolder::SceneObjHolder()'),@($scene,'NameObj* SceneObjHolder::create('),@($scene,'NameObj* SceneObjHolder::getObj('))){
 $text=$spec[0];$b=$text.IndexOf($spec[1]);if($b -lt 0){throw 'Missing original routing anchor'};$o=$text.IndexOf('{',$b);$d=1;$e=$o+1
 while($d){if($text[$e] -eq '{'){$d++}elseif($text[$e] -eq '}'){$d--};$e++};$routing+=$text.Substring($b,$e-$b)
}
Set-Content "$n\src\generated\StageSwitchRoutingFixture.cpp" ($routing -join [Environment]::NewLine)

