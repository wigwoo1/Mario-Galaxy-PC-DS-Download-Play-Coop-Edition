# Resolve one explicitly recorded build; a missing/quarantined file must never
# cause selection of an older executable still present beside it.
function Get-VerifiedNativeRuntime {
 param([Parameter(Mandatory)][string]$RuntimeDirectory)
 $path=Join-Path $RuntimeDirectory 'latest-native-runtime.json'
 if(!(Test-Path -LiteralPath $path)){throw 'Current build manifest missing. Rebuild the diagnostic; no older launcher will be selected.'}
 $record=Get-Content -LiteralPath $path -Raw -ErrorAction Stop | ConvertFrom-Json -ErrorAction Stop
 if($record.Schema-ne 1 -or $record.LauncherName-notmatch '^galaxy-native-[a-f0-9]+\.exe$' -or $record.LauncherSHA256-notmatch '^[a-f0-9]{64}$' -or $record.LibrarySHA256-notmatch '^[a-f0-9]{64}$'){throw 'Invalid current build manifest.'}
 $launcherPath=Join-Path $RuntimeDirectory $record.LauncherName
 $libraryPath=Join-Path $RuntimeDirectory 'galaxy-native.dll'
 if(!(Test-Path -LiteralPath $launcherPath) -or !(Test-Path -LiteralPath $libraryPath)){throw 'Current build artifact missing; inspect build/security results. No older launcher will be selected.'}
 if((Get-FileHash -LiteralPath $launcherPath -Algorithm SHA256).Hash-ne $record.LauncherSHA256 -or (Get-FileHash -LiteralPath $libraryPath -Algorithm SHA256).Hash-ne $record.LibrarySHA256){throw 'Current build artifact hash mismatch. Diagnostic was not started.'}
 [pscustomobject]@{Launcher=(Get-Item -LiteralPath $launcherPath);Library=(Get-Item -LiteralPath $libraryPath);Record=$record}
}

function Assert-NativeDiagnosticOptions {
 param([System.Collections.IDictionary]$Options=([Environment]::GetEnvironmentVariables()))
 $reload=$Options['GALAXY_DIAGNOSTIC_EXPECT_PROFILE']
 if($null-ne $reload){
  if([string]$reload-notmatch '^[1-6]$'){throw 'Reload diagnostic expects one profile number from 1 to 6; diagnostic was not launched.'}
  foreach($name in @('GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT','GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT','GALAXY_DIAGNOSTIC_CREATE_PROFILE','GALAXY_DIAGNOSTIC_SELECT_START_BUTTON')){
   if($null-ne $Options[$name]){throw 'Read-only profile reload cannot select, create, or start a profile. Remove GALAXY_DIAGNOSTIC_EXPECT_PROFILE for a saved-profile stage test; diagnostic was not launched.'}
  }
 }
}

function Assert-NativeRuntimeDiagnosticInput {
 param([Parameter(Mandatory)][string]$RuntimeDirectory,
       [Parameter(Mandatory)]$VerifiedRuntime,
       [bool]$InputRequested=$false)
 if(!$InputRequested){return}
 $property=$VerifiedRuntime.Record.PSObject.Properties['DiagnosticInputEnabled']
 if($property){
  if($property.Value-is [bool] -and $property.Value){return}
  throw 'Scripted controller input was requested, but this build does not enable it. Rebuild with -DiagnosticPressA; diagnostic was not launched.'
 }
 # Legacy manifests did not record the switch. Their retained generated source
 # provides a compatibility check; do not silently run an unverifiable test.
 $source=Join-Path $RuntimeDirectory 'GameSystem-boot-traced.cpp'
 if(!(Test-Path -LiteralPath $source) -or !([IO.File]::ReadAllText($source).Contains('NativeDiagnosticInput::next'))){
  throw 'This legacy build cannot verify scripted controller input. Rebuild with -DiagnosticPressA; diagnostic was not launched.'
 }
}
