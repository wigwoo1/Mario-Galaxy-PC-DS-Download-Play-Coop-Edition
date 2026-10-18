$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Demo\DemoExecutor.cpp" -Raw
$pattern='std::binder2nd<\s*std::mem_fun1_t<\s*void,\s*DemoTalkAnimCtrl,\s*const char\*\s*>,\s*const char\*\s*>\(\s*std::mem_func\(&DemoTalkAnimCtrl::setupStartDemoPart\),\s*(pParam[13])\)'
if([regex]::Matches($s,$pattern).Count -ne 3){throw 'Expected three original talk part adapters'}
$s=[regex]::Replace($s,$pattern,'[$1](DemoTalkAnimCtrl* control){control->setupStartDemoPart($1);}')
$s=[regex]::Replace($s,'std::mem_func\(&([A-Za-z0-9_]+)::([A-Za-z0-9_]+)\)','[]($1* value){value->$2();}')
$s=$s.Replace('_44 = nullptr;', '_44 = 0;').Replace('_48 = nullptr;', '_48 = 0;')
$s=$s.Replace('std::ptr_fun(MR::validateClipping)','[](LiveActor* actor){MR::validateClipping(actor);}')
$s|Set-Content "$n\src\generated\DemoExecutor.cpp"

