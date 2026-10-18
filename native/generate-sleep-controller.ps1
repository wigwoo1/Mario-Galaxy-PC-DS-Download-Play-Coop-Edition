$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\SleepControllerHolder.cpp" -Raw
# Replace removed C++17 mem_func with equivalent member-call lambdas.
$s=$s.Replace('std::mem_func(&SleepController::update)','[](SleepController* controller) { controller->update(); }')
$s=$s.Replace('std::mem_func(&SleepController::initSync)','[](SleepController* controller) { controller->initSync(); }')
Set-Content "$n\src\generated\SleepControllerHolderKernel.cpp" $s
