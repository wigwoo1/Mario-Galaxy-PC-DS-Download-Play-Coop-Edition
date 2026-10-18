$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\RhythmLib\AudMePlayer.cpp" -Raw
$s=$s.Replace('mMeSeq->mNumEntries','nativeMeTableWord(mMeSeq)').Replace('return pStartPos[meId];','return s32(nativeMeTableWord(pStartPos + meId));')
$s='namespace { unsigned int nativeMeTableWord(const void* value){auto p=static_cast<const unsigned char*>(value);return unsigned(p[0])<<24|unsigned(p[1])<<16|unsigned(p[2])<<8|p[3];} }'+[Environment]::NewLine+$s
$start=$s.IndexOf('void AudMePlayingParamsHolder::setResource(');if($start -lt 0){throw 'Missing original params holder'};$s=$s.Substring(0,$start)
Set-Content "$n\src\generated\AudMePlayer.cpp" $s

