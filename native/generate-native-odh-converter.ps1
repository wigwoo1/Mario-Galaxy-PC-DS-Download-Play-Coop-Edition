$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Screen\OdhConverter.cpp" -Raw
$anchor='mIsRequestedCapture(false), mImage(nullptr)'
if(!$s.Contains($anchor)){throw 'Capture initialization anchor missing'}
$s=$s.Replace($anchor,'mIsRequestedCapture(false), mImageSize(0), mImage(nullptr)')
$a=$s.IndexOf('    u32 getOdhImageSize()');$b=$s.IndexOf('    NameObjAdaptor*', $a)
if($a -lt 0 -or $b -lt 0){throw 'Capture image size anchor missing'}
$body=$s.Substring($a,$b-$a).Replace('return nullptr;','return 0;')
$s=$s.Remove($a,$b-$a).Insert($a,$body)
Set-Content "$n\src\generated\OdhConverter.cpp" $s
