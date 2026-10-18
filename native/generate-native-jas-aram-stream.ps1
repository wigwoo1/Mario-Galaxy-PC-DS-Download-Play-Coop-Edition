$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASAramStream.cpp" -Raw
$s='#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#include <cstdint>'+[Environment]::NewLine+$s
$s=$s.Replace('(u32)msg','u32(reinterpret_cast<uintptr_t>(msg))')
$s=$s.Replace('headerLoadTask, &data, 0xc', 'headerLoadTask, &data, sizeof(TaskData)').Replace('firstLoadTask, data, 0xc', 'firstLoadTask, data, sizeof(TaskData)').Replace('firstLoadTask, &data, 0xc', 'firstLoadTask, &data, sizeof(TaskData)')
$s='#include <Native/JasStreamEndian.hpp>'+[Environment]::NewLine+$s
$s=$s.Replace('DVDReadPrio(&mDvdFileInfo, sReadBuffer, sizeof(Header), 0, 1) < 0','DVDReadPrio(&mDvdFileInfo, sReadBuffer, sizeof(Header), 0, 1) != sizeof(Header)')
$s=$s.Replace('Header* header = (Header*)sReadBuffer;', 'Header decoded{};'+[Environment]::NewLine+'    if (!Native::decodeJasStreamHeader(sReadBuffer, sizeof(Header), decoded) || !Native::validJasStreamBlockSize(sBlockSize, decoded.format) || decoded.channels > sChannelMax || (param_0 / sBlockSize) / decoded.channels < 2) return false;'+[Environment]::NewLine+'    Header* header = &decoded;')
$s=$s.Replace('u32 size = sBlockSize * mChannelNum + sizeof(BlockHeader);', 'const uint64_t fullSize = uint64_t(sBlockSize) * mChannelNum + sizeof(BlockHeader);'+[Environment]::NewLine+'    const uint64_t start = uint64_t(mBlock) * fullSize + sizeof(Header);'+[Environment]::NewLine+'    if (fullSize > UINT32_MAX || start > mDvdFileInfo.length) return false;'+[Environment]::NewLine+'    u32 size = u32(fullSize);')
$s=$s.Replace('u32 offset = mBlock * size + sizeof(Header);','u32 offset = u32(start);')
$s=$s.Replace('if (DVDReadPrio(&mDvdFileInfo, sReadBuffer, size, offset, 1) < 0)', 'if (size > fullSize || size > mDvdFileInfo.length - offset || DVDReadPrio(&mDvdFileInfo, sReadBuffer, size, offset, 1) != size)')
$s=$s.Replace('BlockHeader* bhead = (BlockHeader*)sReadBuffer;', 'BlockHeader decodedBlock{};'+[Environment]::NewLine+'    if (!Native::decodeJasStreamBlock(sReadBuffer, size, mChannelNum, sBlockSize, decodedBlock)) return false;'+[Environment]::NewLine+'    BlockHeader* bhead = &decodedBlock;')
$s='#include <stdexcept>'+[Environment]::NewLine+$s
$s=$s.Replace('void JASAramStream::initSystem(u32 block_size, u32 channel_max) {', 'void JASAramStream::initSystem(u32 block_size, u32 channel_max) {'+[Environment]::NewLine+'    u32 readBytes; if (!Native::jasStreamReadBufferSize(block_size, channel_max, readBytes)) throw std::invalid_argument("invalid audio stream buffer dimensions");')
$s=$s.Replace('u8[(block_size + 0x20) * channel_max]', 'u8[readBytes]')
$s=[regex]::Replace($s,'while \(r28 > loop_end_block\) \{\s*r28 -= loop_end_block;\s*r28 \+= loop_start_block;\s*\}', 'if (!Native::jasStreamLoopLookAhead(uint64_t(mBlock) + _108 - 1, loop_start_block, loop_end_block, r28)) return false;')
$s=$s.Replace('u32 sp08 = _148 + _10C * sBlockSize;', 'u32 channelAddresses[6];'+[Environment]::NewLine+'    for (unsigned ch = 0; ch < mChannelNum; ++ch) if (!Native::jasStreamTransferAddress(_148, _10C, sBlockSize, _160, mChannelNum, ch, bhead->_4, channelAddresses[ch])) return false;')
$s=$s.Replace('sp08 + sBlockSize * _160 * i', 'channelAddresses[i]')
Set-Content "$n\src\generated\JASAramStream.cpp" $s







