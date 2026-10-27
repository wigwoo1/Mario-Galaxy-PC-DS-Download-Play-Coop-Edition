param([Parameter(Mandatory)][ValidatePattern('^[a-zA-Z0-9_-]+$')][string]$Prefix,
      [ValidateRange(1,3600)][int]$Seconds=40,
      [ValidateRange(1,100000)][int]$CaptureFrame=240,
      [switch]$CaptureDump,
      [string]$FramebufferCompletionMarker,
      [int]$ExistingProcessId=0)
$ErrorActionPreference='Stop'
$runtime=Join-Path $env:TEMP 'galaxy-dspboot-link-verification'
$evidence=Join-Path $PSScriptRoot 'build\startup-verification-20261014'
New-Item -ItemType Directory -Force -Path $evidence | Out-Null
. (Join-Path $PSScriptRoot 'native-runtime-manifest.ps1')
Assert-NativeDiagnosticOptions
$verified=Get-VerifiedNativeRuntime -RuntimeDirectory $runtime
$inputRequested=$false
foreach($name in @('GALAXY_DIAGNOSTIC_PRESS_AB','GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT','GALAXY_DIAGNOSTIC_CREATE_PROFILE','GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT','GALAXY_DIAGNOSTIC_SELECT_START_BUTTON')){
 if($null-ne [Environment]::GetEnvironmentVariable($name)){$inputRequested=$true}
}
Assert-NativeRuntimeDiagnosticInput -RuntimeDirectory $runtime -VerifiedRuntime $verified -InputRequested $inputRequested
$launcher=$verified.Launcher
$traces=@(@('galaxy-native-boot.log','boot.log'),@('galaxy-native-firstchance.log','firstchance.log'),@('galaxy-native-launcher-error.log','error.log'))
if(!$ExistingProcessId){foreach($trace in $traces){$path=Join-Path $env:TEMP $trace[0];if(Test-Path -LiteralPath $path){Clear-Content -LiteralPath $path}}}
$started=Get-Date
$previousTmp=$env:TMP
try{
 $env:TMP=$env:TEMP # GetTempPathW prioritizes TMP; keep child trace paths with this probe's TEMP.
 if($ExistingProcessId){
  $process=Get-Process -Id $ExistingProcessId
  if($process.MainModule.FileName-ne $launcher.FullName){throw 'Existing PID does not match the current diagnostic launcher.'}
  $started=$process.StartTime
 }else{
  $process=Start-Process -FilePath $launcher.FullName -ArgumentList '"D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs"' -WorkingDirectory $runtime -WindowStyle Hidden -RedirectStandardError (Join-Path $evidence "$Prefix.stderr.log") -RedirectStandardOutput (Join-Path $evidence "$Prefix.stdout.log") -PassThru
 }
}finally{$env:TMP=$previousTmp}
Write-Output "Started owned diagnostic PID=$($process.Id) launcher=$($launcher.Name)"
$errorPath=Join-Path $env:TEMP 'galaxy-native-launcher-error.log'
$caught=$false;$completedCapture=$false
while(((Get-Date)-$started).TotalSeconds -lt $Seconds){
 if($process.WaitForExit(1000)){break}
 if((Test-Path -LiteralPath $errorPath) -and (Get-Item -LiteralPath $errorPath).Length){$caught=$true;break}
 if($FramebufferCompletionMarker -and (Test-Path -LiteralPath $FramebufferCompletionMarker)){
  $markerStream=$null;$markerReader=$null
  try{
   $markerStream=[IO.File]::Open($FramebufferCompletionMarker,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::ReadWrite)
   $markerReader=[IO.StreamReader]::new($markerStream)
   if($markerReader.ReadToEnd().Contains('FRAME COMPLETE')){$completedCapture=$true;break}
  }catch [IO.IOException]{}finally{if($markerReader){$markerReader.Dispose()}elseif($markerStream){$markerStream.Dispose()}}
 }
}
$result=@("Launcher=$($launcher.Name)", ('Elapsed={0:F1}s' -f ((Get-Date)-$started).TotalSeconds))
if(!$process.HasExited){
 $process.Refresh()
 $result+=('Memory at diagnostic stop: private={0} working={1} peak-working={2} bytes' -f $process.PrivateMemorySize64,$process.WorkingSet64,$process.PeakWorkingSet64)
 try{
  $memory=Get-CimInstance Win32_PerfFormattedData_PerfOS_Memory -OperationTimeoutSec 3 -ErrorAction Stop
  $result+=('System memory at diagnostic stop: available={0}MiB committed={1} limit={2} bytes' -f $memory.AvailableMBytes,$memory.CommittedBytes,$memory.CommitLimit)
 }catch{$result+='System memory counters unavailable at diagnostic stop.'}
 $module=$process.Modules | Where-Object ModuleName -EQ 'galaxy-native.dll' | Select-Object -First 1
 if($module){$result+=('Native DLL base=0x{0:X}' -f $module.BaseAddress.ToInt64())}
 if($CaptureDump){
  if(-not ('GalaxyProbeDump' -as [type])){Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class GalaxyProbeDump {
 [DllImport("dbghelp.dll", SetLastError=true)]
 public static extern bool MiniDumpWriteDump(IntPtr process, uint pid, IntPtr file, uint type, IntPtr exception, IntPtr userStreams, IntPtr callback);
}
'@}
  $dumpPath=Join-Path $evidence "$Prefix.dmp"
  $dumpFile=[IO.File]::Create($dumpPath)
  try{
   if(-not [GalaxyProbeDump]::MiniDumpWriteDump($process.Handle,[uint32]$process.Id,$dumpFile.SafeFileHandle.DangerousGetHandle(),0,[IntPtr]::Zero,[IntPtr]::Zero,[IntPtr]::Zero)){throw "Startup dump failed: Win32 error $([Runtime.InteropServices.Marshal]::GetLastWin32Error())"}
   $result+="Live thread dump=$dumpPath"
  }finally{$dumpFile.Dispose()}
 }
 Stop-Process -Id $process.Id
 $result+= $(if($caught){'Caught runtime error; stopped its launcher.'}elseif($completedCapture){'Stopped after requested framebuffer observation completed; inspect images before claiming playability.'}else{"Stopped at the $Seconds-second diagnostic limit; inspect traces before claiming stability."})
}else{$result+="Exit=$($process.ExitCode)"}
$result | Set-Content -LiteralPath (Join-Path $evidence "$Prefix.result.txt")
$result | Write-Output
foreach($trace in $traces){$path=Join-Path $env:TEMP $trace[0];if(Test-Path -LiteralPath $path){Copy-Item -LiteralPath $path -Destination (Join-Path $evidence "$Prefix.$($trace[1])")}}
$capture=Join-Path $runtime "retail-frame-$CaptureFrame-xfb.bmp"
if((Test-Path -LiteralPath $capture) -and (Get-Item -LiteralPath $capture).LastWriteTime -ge $started){Copy-Item -LiteralPath $capture -Destination (Join-Path $evidence "$Prefix-frame$CaptureFrame.bmp")}
Get-Content (Join-Path $evidence "$Prefix.boot.log") | Select-String 'XFB capture|Logo state' | Select-Object -Last 8 | ForEach-Object Line
if($caught){Get-Content -LiteralPath $errorPath}
