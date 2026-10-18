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
