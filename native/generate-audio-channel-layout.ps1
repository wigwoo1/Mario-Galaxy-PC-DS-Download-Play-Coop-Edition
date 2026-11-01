$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASChannel.hpp'
$text=[IO.File]::ReadAllText($source)
$old=@'
            u8 upper : 8;
            u8 lower0 : 4;
            u8 lower1 : 4;
'@
$old=$old.Replace("`r`n","`n");$text=$text.Replace("`r`n","`n")
if(($text.Split(@($old),[StringSplitOptions]::None)).Count-ne 2){throw 'Original MixConfig layout anchor changed'}
$replacement=@'
            // Packed values retain the original bus:8 / modifiers:4:4 meaning.
            // Native little-endian bitfields allocate from the low bit first.
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            u16 lower1 : 4;
            u16 lower0 : 4;
            u16 upper : 8;
#else
            u8 upper : 8;
            u8 lower0 : 4;
            u8 lower1 : 4;
#endif
'@
$target=Join-Path $n 'include\JSystem\JAudio2\JASChannel.hpp'
[IO.File]::WriteAllText($target,"// Generated from Petari by generate-audio-channel-layout.ps1; preserve packed mixer semantics.`n"+$text.Replace($old,$replacement.Replace("`r`n","`n")),[Text.UTF8Encoding]::new($false))
Write-Host 'Generated native JASChannel packed mixer layout'
