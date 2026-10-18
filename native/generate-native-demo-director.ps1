$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\DemoUtil.cpp" -Raw
$start=$s.IndexOf('    LiveActor* getTalkingActor() {');$end=$s.IndexOf('    bool isDemoPartTalk(', $start)
if($start -lt 0 -or $end -lt 0){throw 'Original talking actor function missing'}
$function=$s.Substring($start,$end-$start).Replace('return false;', 'return nullptr;')
$s=$s.Substring(0,$start)+$function+$s.Substring($end)
$s|Set-Content "$n\src\generated\DemoUtil.cpp"
$s=Get-Content "$p\Petari\src\Game\Demo\DemoDirector.cpp" -Raw
if(!$s.Contains('info.mFrameType == nullptr')){throw 'Original cinema enum comparison missing'}
$s=$s.Replace('info.mFrameType == nullptr','info.mFrameType == DemoStartInfo::CinemaFrameType_0')
$s|Set-Content "$n\src\generated\DemoDirector.cpp"
