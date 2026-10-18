$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\NPC\NPCActor.cpp" -Raw
$q=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JGeometry\TVec.hpp" -Raw
$a=$q.IndexOf('void getEuler(TVec3< T >& rDest)');if($a -lt 0){throw 'Missing original quaternion conversion'}
$b=$q.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($q[$e] -eq '{'){$d++}elseif($q[$e] -eq '}'){$d--};$e++}
$body=$q.Substring($b,$e-$b).Replace('this->','_A0.').Replace('rDest','euler').Replace('TUtil< f32 >','JGeometry::TUtil< f32 >')
$s=$s.Replace('&calcJointScale,','&NPCActor::calcJointScale,').Replace('_A0.getEuler(euler);',$body)
Set-Content "$n\src\generated\NPCActor.cpp" $s
