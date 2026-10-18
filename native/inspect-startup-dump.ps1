param([Parameter(Mandatory)][string]$Path,[ValidateRange(8,1048576)][int]$StackBytes=2048)
$ErrorActionPreference='Stop'
$dumpBytes=[IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path))
function ReadDumpU32([int]$offset){[BitConverter]::ToUInt32($dumpBytes,$offset)}
function ReadDumpU64([int]$offset){[BitConverter]::ToUInt64($dumpBytes,$offset)}
if((ReadDumpU32 0) -ne 0x504d444d){throw 'Not a Windows minidump.'}
$streams=@{}
$directory=ReadDumpU32 12
for($i=0;$i -lt (ReadDumpU32 8);$i++){$entry=$directory+$i*12;$streams[(ReadDumpU32 $entry)]=(ReadDumpU32 ($entry+8))}
if(!$streams.ContainsKey([uint32]3)-or !$streams.ContainsKey([uint32]4)){throw 'Dump requires thread and module streams.'}
$modules=@()
$moduleList=$streams[[uint32]4]
for($i=0;$i -lt (ReadDumpU32 $moduleList);$i++){
 $entry=$moduleList+4+$i*108;$nameRva=ReadDumpU32 ($entry+20)
 $modules+= [pscustomobject]@{Base=(ReadDumpU64 $entry);Size=(ReadDumpU32 ($entry+8));Name=[Text.Encoding]::Unicode.GetString($dumpBytes,$nameRva+4,(ReadDumpU32 $nameRva))}
}
$nativeModule=$modules|Where-Object {[IO.Path]::GetFileName($_.Name)-eq 'galaxy-native.dll'}|Select-Object -First 1
if(!$nativeModule){throw 'Dump does not contain galaxy-native.dll.'}
'Native module base=0x{0:X} size=0x{1:X}' -f $nativeModule.Base,$nativeModule.Size
$threadList=$streams[[uint32]3]
for($i=0;$i -lt (ReadDumpU32 $threadList);$i++){
 $entry=$threadList+4+$i*48;$tid=ReadDumpU32 $entry;$contextRva=ReadDumpU32 ($entry+44)
 $rip=ReadDumpU64 ($contextRva+248);$rsp=ReadDumpU64 ($contextRva+152)
 $ripModule=$modules|Where-Object {$rip-ge $_.Base-and $rip-lt ($_.Base+$_.Size)}|Select-Object -First 1
 'Thread {0}: RIP=0x{1:X} RSP=0x{2:X} module={3}' -f $tid,$rip,$rsp,([IO.Path]::GetFileName($ripModule.Name))
 $stackAddress=ReadDumpU64 ($entry+24);$stackSize=ReadDumpU32 ($entry+32);$stackRva=ReadDumpU32 ($entry+36)
 # These are candidate return addresses, not a debugger-unwound call stack.
 $start=[int][Math]::Max(0,([int64]$rsp-[int64]$stackAddress))
 $end=[int][Math]::Min($stackSize,$start+$StackBytes)
 for($offset=$start;$offset+8-le $end;$offset+=8){
  $value=ReadDumpU64 ($stackRva+$offset)
  if($value-ge $nativeModule.Base-and $value-lt ($nativeModule.Base+$nativeModule.Size)){
   '  stack+0x{0:X}: native RVA=0x{1:X} address=0x{2:X}' -f ($offset-$start),($value-$nativeModule.Base),$value
  }
 }
}
