$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\include\Game\RhythmLib\AudMeSeqReader.hpp" -Raw
$s=$s.Replace('return *(u16*)(mSeqBuff + addr);','return (u16(mSeqBuff[addr]) << 8) | mSeqBuff[addr + 1];')
$s=$s.Replace('return (*(u32*)(mSeqBuff + addr - 1)) & 0xffffff;','return (u32(mSeqBuff[addr]) << 16) | (u32(mSeqBuff[addr + 1]) << 8) | mSeqBuff[addr + 2];')
$s=$s.Replace('return *(u32*)(mSeqBuff + addr);','return (u32(mSeqBuff[addr]) << 24) | (u32(mSeqBuff[addr + 1]) << 16) | (u32(mSeqBuff[addr + 2]) << 8) | mSeqBuff[addr + 3];')
New-Item -ItemType Directory -Force "$n\include\Game\RhythmLib" | Out-Null
Set-Content "$n\include\Game\RhythmLib\AudMeSeqReader.hpp" $s
