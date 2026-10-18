$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$bytes=[IO.File]::ReadAllBytes("$n\build\reference-main.dol")
function ReadWord([int]$at){return [uint32](([uint64]$bytes[$at]*16777216)+([uint64]$bytes[$at+1]*65536)+([uint64]$bytes[$at+2]*256)+$bytes[$at+3])}
function Offset([uint32]$address){for($i=0;$i -lt 18;$i++){$base=ReadWord (0x48+$i*4);$size=ReadWord (0x90+$i*4);if($address -ge $base -and [uint64]$address -lt [uint64]$base+$size){return (ReadWord ($i*4))+$address-$base}}throw 'Address outside DOL'}
$found=@()
for($i=0;$i -lt 7;$i++){$start=ReadWord ($i*4);$size=ReadWord (0x90+$i*4);for($at=$start;$at+8 -le $start+$size;$at+=4){if($bytes[$at] -ne 0x3d -or $bytes[$at+1] -ne 0xa0 -or ($bytes[$at+4] -ne 0x39 -and $bytes[$at+4] -ne 0x61) -or $bytes[$at+5] -ne 0xad){continue};$hi=(ReadWord $at)-band 65535;$lo=(ReadWord ($at+4))-band 65535;if($bytes[$at+4] -eq 0x39 -and $lo -ge 32768){$lo-=65536};$base=[uint32]([long]$hi*65536+$lo);$found+=$base}}
$found|ForEach-Object {'r13 base candidate: 0x{0:x8}' -f $_}
foreach($base in $found){$stackAddress=[uint32]([long]$base+0x8f3c-65536);$messageAddress=[uint32]([long]$base+0x8f40-65536);[pscustomobject]@{R13=('0x{0:x8}' -f $base);StackAddress=('0x{0:x8}' -f $stackAddress);StackSize=(ReadWord (Offset $stackAddress));MessageAddress=('0x{0:x8}' -f $messageAddress);MessageCount=(ReadWord (Offset $messageAddress))}|ConvertTo-Json|Set-Content "$n\build\aram-startup-globals.json";Get-Content "$n\build\aram-startup-globals.json"}

