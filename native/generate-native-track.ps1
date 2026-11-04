$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$header=[IO.File]::ReadAllText((Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASTrack.hpp')).Replace("`r`n","`n")
$begin=$header.IndexOf('    struct TList {');$end=$header.IndexOf('    struct MoveParam_', $begin)
if($begin-lt 0-or $end-le $begin){throw 'Original nested track-list declaration changed'}
$list=$header.Substring($begin,$end-$begin).Trim()
$offset='typedef JGadget::TLinkList< JASTrack, -0x248 > TrackList;'
if([regex]::Matches($list,[regex]::Escape($offset)).Count-ne 1){throw 'Original track-list node offset anchor changed'}
$list=$list.Replace('struct TList {','struct JASTrack::TList {').Replace($offset,'typedef JGadget::TLinkList< JASTrack, -static_cast<int>(offsetof(JASTrack, mNode)) > TrackList;')
# Compute the node offset only after JASTrack is complete; the nested type and
# every data member retain their original layout and public API.
$header=$header.Remove($begin,$end-$begin).Insert($begin,"    struct TList;`n`n")
$weak='JASChannel* channelStart(JASTrack::TChannelMgr*, u32, u32, u32) ATTRIBUTE_WEAK;'
if([regex]::Matches($header,[regex]::Escape($weak)).Count-ne 1){throw 'Original track note provider declaration changed'}
$header=$header.Replace($weak,'JASChannel* channelStart(JASTrack::TChannelMgr*, u32, u32, u32);')
[IO.File]::WriteAllText((Join-Path $n 'include\JSystem\JAudio2\JASTrack.hpp'),"#include <cstddef>`n"+$header+"`n"+$list+"`n",[Text.UTF8Encoding]::new($false))
# Native code has one selected bank dispatcher. A weak PE reference may stay
# null even with the weak definition present; require this game-facing API.
$bankHeader=[IO.File]::ReadAllText((Join-Path $p 'Petari\libs\JSystem\include\JSystem\JAudio2\JASBank.hpp'))
$weakBank='void*) ATTRIBUTE_WEAK;'
if([regex]::Matches($bankHeader,[regex]::Escape($weakBank)).Count-ne 1){throw 'Original bank note declaration changed'}
$bankHeader=$bankHeader.Replace($weakBank,'void*);')
[IO.File]::WriteAllText((Join-Path $n 'include\JSystem\JAudio2\JASBank.hpp'),$bankHeader,[Text.UTF8Encoding]::new($false))
$source=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JASTrack.cpp'))
# Data has native pointers: its Wii24-byte prefix omits the x64 scale/bias.
foreach($copy in @(
 @{Old='memcpy(mOscParam, &sEnvOsc, 0x18);';New='memcpy(mOscParam, &sEnvOsc, sizeof(sEnvOsc));';Count=2},
 @{Old='memcpy(mOscParam + 1, &sPitchEnvOsc, 0x18);';New='memcpy(mOscParam + 1, &sPitchEnvOsc, sizeof(sPitchEnvOsc));';Count=1}
)){
 if([regex]::Matches($source,[regex]::Escape($copy.Old)).Count-ne $copy.Count){throw 'Original track oscillator copy anchors changed'}
 $source=$source.Replace($copy.Old,$copy.New)
}
# The game overrides this weak JAudio entry point in OverwriteJAudio.cpp.
# Retain its complete original method; an unresolved weak symbol previously
# let the DLL link while the first actual sequence note called address zero.
$game=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\Game\AudioLib\OverwriteJAudio.cpp'))
$start=$game.IndexOf('JASChannel* JASTrack::channelStart(')
$end=$game.IndexOf('void JASWaveArc::setFileName(', $start)
if($start-lt 0-or $end-le $start){throw 'Original game track note provider anchors changed'}
$source+="`n#include <JSystem/JSupport/JSupport.hpp>`n#include <JSystem/JAudio2/JASBankTable.hpp>`n#include <JSystem/JAudio2/JASBank.hpp>`n#include <JSystem/JAudio2/JASBasicInst.hpp>`n#include <Game/AudioLib/AudUtil.hpp>`n"+$game.Substring($start,$end-$start)
# The base native support already supplies AudUtil's two numeric helpers.
# Add only this original missing policy method, avoiding duplicate definitions.
$util=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\Game\AudioLib\AudUtil.cpp'))
$policyStart=$util.IndexOf('    bool isProperCurrentStageOrAudioDebugMode() {')
$policyEnd=$util.IndexOf('    }', $policyStart)
if($policyStart-lt 0-or $policyEnd-le $policyStart){throw 'Original audio policy function anchors changed'}
$source+="`nnamespace AudUtil {`n"+$util.Substring($policyStart,$policyEnd-$policyStart+5)+"`n}`n"
$bankHeaderHash=(Get-FileHash -LiteralPath (Join-Path $n 'include\JSystem\JAudio2\JASBank.hpp') -Algorithm SHA256).Hash
$bankSource=[IO.File]::ReadAllText((Join-Path $p 'Petari\src\JSystem\JAudio2\JASBank.cpp'))
[IO.File]::WriteAllText((Join-Path $n 'src\generated\JASBank.cpp'),"// Native bank header SHA256: $bankHeaderHash`n#include <Native/HeapPlacement.hpp>`n"+$bankSource,[Text.UTF8Encoding]::new($false))
$headerHash=(Get-FileHash -LiteralPath (Join-Path $n 'include\JSystem\JAudio2\JASTrack.hpp') -Algorithm SHA256).Hash
[IO.File]::WriteAllText((Join-Path $n 'src\generated\JASTrack.cpp'),"// Native track header SHA256: $headerHash`n// Native bank header SHA256: $bankHeaderHash`n#include <Native/HeapPlacement.hpp>`n"+$source+"`nstatic_assert(sizeof(JASTrack)==808, `"Native track layout mismatch`");`nstatic_assert(offsetof(JASTrack,mNode)==792, `"Native node offset mismatch`");`n",[Text.UTF8Encoding]::new($false))
