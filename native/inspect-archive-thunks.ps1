$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$bytes=[IO.File]::ReadAllBytes("$n\build\reference-main.dol")
function ReadWord([int]$at){return [uint32](([uint64]$bytes[$at]*16777216)+([uint64]$bytes[$at+1]*65536)+([uint64]$bytes[$at+2]*256)+$bytes[$at+3])}
$found=@()
for($section=0;$section -lt 7;$section++){
$start=ReadWord ($section*4);$address=ReadWord (0x48+$section*4);$size=ReadWord (0x90+$section*4)
for($offset=$start;$offset+16 -le $start+$size;$offset+=4){
if((ReadWord $offset) -eq 2172846080L -and ((ReadWord ($offset+4)) -band 4294901760L) -eq 2173435904L -and (ReadWord ($offset+8)) -eq 0x7d8903a6 -and (ReadWord ($offset+12)) -eq 0x4e800420){
$entry=[pscustomobject]@{Address=('0x{0:x8}' -f ($address+$offset-$start));FileOffset=$offset;VirtualOffset=((ReadWord ($offset+4)) -band 65535)};$found+=$entry
}
}
}
$found|ConvertTo-Json|Set-Content "$n\build\archive-virtual-thunk-candidates.json"
$found|Where-Object {$_.Address -like '0x8027*'}|Format-Table
"Total verified four-instruction virtual-tail-call candidates: $($found.Count)"


for($section=7;$section -lt 18;$section++){$start=ReadWord ($section*4);$address=ReadWord (0x48+$section*4);$size=ReadWord (0x90+$section*4);for($at=$start;$at+4 -le $start+$size;$at+=4){if($bytes[$at] -eq 0x80 -and $bytes[$at+1] -eq 0x27 -and (ReadWord $at) -eq 2150061528L){"Candidate reference at {0:x8}" -f ($address+$at-$start);for($relative=-60;$relative -le 16;$relative+=4){"{0:x2}: {1:x8}" -f ($relative+60),(ReadWord ($at+$relative))}}}}
