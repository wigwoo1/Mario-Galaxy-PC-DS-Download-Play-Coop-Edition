$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$bytes=[IO.File]::ReadAllBytes("$n\build\reference-main.dol")
function ReadWord([int]$at){return [uint32](([uint64]$bytes[$at]*16777216)+([uint64]$bytes[$at+1]*65536)+([uint64]$bytes[$at+2]*256)+$bytes[$at+3])}
function Offset([uint32]$address){for($i=0;$i -lt 7;$i++){$base=ReadWord (0x48+$i*4);$size=ReadWord (0x90+$i*4);if($address -ge $base -and [uint64]$address -lt [uint64]$base+$size){return (ReadWord ($i*4))+$address-$base}}throw 'Address outside text'}
$calls=@()
for($address=0x803a5604L;$address -lt 0x803a5710L;$address+=4){$w=ReadWord (Offset $address);if(($w -shr 26) -ne 18 -or ($w -band 3) -ne 1){continue};$delta=[long]($w -band 0x3fffffc);if($delta -band 0x2000000){$delta-=0x4000000};$target=$address+$delta;$instructions=@();$start=Offset $target;for($j=0;$j -lt 64;$j++){$instructions+=('0x{0:x8}' -f (ReadWord ($start+$j*4)))};$calls+=[pscustomobject]@{Call=('0x{0:x8}' -f $address);Target=('0x{0:x8}' -f $target);Words=$instructions}}
$calls|ConvertTo-Json -Depth 4|Set-Content "$n\build\aram-constructor-call-targets.json"
$calls|Format-Table Call,Target,@{Name='FirstWords';Expression={$_.Words[0..3] -join ' '}}

