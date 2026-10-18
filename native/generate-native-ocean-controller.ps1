$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\OceanHomeMapCtrl.cpp" -Raw
# Exact bytes recovered from verified North American DOL and confirmed PPC call-site references.
$quoted=@([regex]::Matches($s,'"[^"\r\n]*"') | Select-Object -Last 7)
if([int]$quoted[1].Value[1] -ne 0x30aa -or [int]$quoted[5].Value[1] -ne 0x6d77){throw 'Unexpected original Japanese ocean name ordering'}
$names=@{
 $quoted[1].Value='8349815b835683838393837a815b8380926e8c6090a78ce4'
 $quoted[2].Value='8349815b835683838393838a8393834f81699085928697704c6f77816a'
 $quoted[5].Value='8a43976d837a815b8380986690af'
 $quoted[6].Value='8349815b835683838393838a8393834f986690af'
}
foreach($name in $names.Keys){$old=$name;if(!$s.Contains($old)){throw "Original ocean name missing: $name"};$hex=$names[$name];$literal='"';for($i=0;$i -lt $hex.Length;$i+=2){$literal+='\x'+$hex.Substring($i,2)};$literal+='"';$s=$s.Replace($old,$literal)}
$s|Set-Content "$n\src\generated\OceanHomeMapCtrl.cpp"

