$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$bytes=[IO.File]::ReadAllBytes("$n\build\reference-main.dol")
function ReadWord([int]$at){return [uint32](([uint64]$bytes[$at]*16777216)+([uint64]$bytes[$at+1]*65536)+([uint64]$bytes[$at+2]*256)+$bytes[$at+3])}
$found=@()
for($section=0;$section -lt 7;$section++){
 $start=ReadWord ($section*4);$address=ReadWord (0x48+$section*4);$size=ReadWord (0x90+$section*4)
 for($at=$start;$at+4 -le $start+$size;$at+=4){
  $word=ReadWord $at
  if(($word -shr 26) -ne 36 -or ($word -band 65535) -ne 0x94){continue}
  $base=($word -shr 16) -band 31;$writes=@{};$instructions=@()
  for($pos=[Math]::Max($start,$at-0x10c);$pos -le $at;$pos+=4){$w=ReadWord $pos;if(($w -shr 26) -eq 36 -and (($w -shr 16) -band 31) -eq $base){$field=$w -band 65535;if($field -in @(0x7c,0x80,0x84,0x88,0x8c,0x90,0x94)){$writes[$field]=$pos}}}
  if($writes.Count -lt 7){continue}
  for($pos=[Math]::Max($start,$at-0x10c);$pos -le [Math]::Min($start+$size-4,$at+0x40);$pos+=4){$instructions+=[pscustomobject]@{Address=('0x{0:x8}' -f ($address+$pos-$start));Word=('0x{0:x8}' -f (ReadWord $pos))}}
  $found+=[pscustomobject]@{LastFieldStore=('0x{0:x8}' -f ($address+$at-$start));BaseRegister=$base;Instructions=$instructions}
 }
}
$found|ConvertTo-Json -Depth 5|Set-Content "$n\build\aram-constructor-candidates.json"
$found|Select-Object LastFieldStore,BaseRegister|Format-Table
"Candidates matching all seven ARAM field stores: $($found.Count)"
