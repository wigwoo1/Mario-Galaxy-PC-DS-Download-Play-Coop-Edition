$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'native-runtime-manifest.ps1')
$root=Join-Path $PSScriptRoot ('build\runtime-manifest-fixture-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $root | Out-Null
$latest=Join-Path $root 'galaxy-native-a123.exe';$older=Join-Path $root 'galaxy-native-b123.exe';$library=Join-Path $root 'galaxy-native.dll';$manifest=Join-Path $root 'latest-native-runtime.json'
[IO.File]::WriteAllText($latest,'not executable: fixture latest')
[IO.File]::WriteAllText($older,'not executable: fixture older')
[IO.File]::WriteAllText($library,'not executable: fixture library')
$record=@{Schema=1;LauncherName='galaxy-native-a123.exe';LauncherSHA256=(Get-FileHash $latest).Hash;LibrarySHA256=(Get-FileHash $library).Hash}
function SaveManifest {$record | ConvertTo-Json | Set-Content -LiteralPath $manifest}
function ExpectFailure([scriptblock]$Action,[string]$Fragment){$failed=$false;try{& $Action | Out-Null}catch{$failed=$_.Exception.Message.Contains($Fragment);if(!$failed){throw}};if(!$failed){throw "Expected rejection containing: $Fragment"}}
ExpectFailure {Get-VerifiedNativeRuntime $root} 'manifest missing'
SaveManifest
$result=Get-VerifiedNativeRuntime $root
if($result.Launcher.FullName-ne $latest){throw 'Wrong exact launcher selected'}
# A newer timestamp must not override the explicit recorded build.
(Get-Item $older).LastWriteTime=(Get-Date).AddHours(1)
if((Get-VerifiedNativeRuntime $root).Launcher.FullName-ne $latest){throw 'Timestamp overrode exact build identity'}
Remove-Item -LiteralPath $latest
ExpectFailure {Get-VerifiedNativeRuntime $root} 'artifact missing'
[IO.File]::WriteAllText($latest,'changed launcher')
ExpectFailure {Get-VerifiedNativeRuntime $root} 'hash mismatch'
$record.LauncherSHA256=(Get-FileHash $latest).Hash;SaveManifest
[IO.File]::WriteAllText($library,'changed library')
ExpectFailure {Get-VerifiedNativeRuntime $root} 'hash mismatch'
$record.LibrarySHA256=(Get-FileHash $library).Hash;$record.LauncherName='..\other.exe';SaveManifest
ExpectFailure {Get-VerifiedNativeRuntime $root} 'Invalid current build manifest'
$record.LauncherName='galaxy-native-a123.exe';$record.Schema=2;SaveManifest
ExpectFailure {Get-VerifiedNativeRuntime $root} 'Invalid current build manifest'
Write-Output 'PASS exact runtime identity: valid pair, timestamp ignored, missing manifest/artifact rejected without fallback, launcher/library hash mismatch and invalid path/schema rejected; no fixture executed'
