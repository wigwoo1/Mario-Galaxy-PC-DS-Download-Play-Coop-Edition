$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\JointUtil.cpp" -Raw
$end=$s.IndexOf('    void copyJointScale(');if($end -lt 0){throw 'Original joint access boundary missing'}
$parent=$s.IndexOf('    J3DJoint* searchChildJoint(');$parentEnd=$s.IndexOf('};  // namespace MR',$parent)
if($parent -lt 0 -or $parentEnd -le $parent){throw 'Original parent joint anchors missing'}
$trans=$s.IndexOf('    f32 getJointTransX(');if($trans -lt 0 -or $trans -ge $parent){throw 'Original translation getters missing'}
Set-Content "$n\src\generated\JointAccessKernel.cpp" ($s.Substring(0,$end)+$s.Substring($trans,$parent-$trans)+$s.Substring($parent,$parentEnd-$parent)+'}')

