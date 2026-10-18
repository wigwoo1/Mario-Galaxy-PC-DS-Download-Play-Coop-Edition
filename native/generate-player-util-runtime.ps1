$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Util\PlayerUtil.cpp")
$signature='TVec3f* getPlayerPos()'
if($source.IndexOf($signature) -lt 0 -or $source.IndexOf($signature,$source.IndexOf($signature)+$signature.Length) -ge 0){throw 'Expected one original PlayerUtil::getPlayerPos definition'}
[System.IO.File]::WriteAllText("$n\src\generated\PlayerUtilRuntime.cpp",$source,[System.Text.UTF8Encoding]::new($false))
