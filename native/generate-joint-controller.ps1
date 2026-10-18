$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\LiveActor\IKJointCtrl.cpp" -Raw
foreach($method in @('disableCallBack','enableCallBack','endCtrl','setCallBackFunction')){
 $s=$s.Replace("std::mem_fun(&IKJointCtrl::$method)","[](IKJointCtrl* ctrl){ctrl->$method();}")
}
Set-Content "$n\src\generated\IKJointCtrlKernel.cpp" $s
