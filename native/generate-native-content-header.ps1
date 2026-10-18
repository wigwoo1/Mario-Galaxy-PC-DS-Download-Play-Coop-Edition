$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\BinaryDataContentAccessor.cpp" -Raw
foreach($variable in @('numAttributes','streamDataSize','hash','offset','attributeNum','dataSize')){
 $s=$s.Replace("mStream.write(&$variable, sizeof(u16));","nativeContentWriteU16(mStream, $variable);")
}
$s=$s.Replace('*reinterpret_cast< u16* >(mData + 0)','nativeContentReadU16(mData)').Replace('*reinterpret_cast< u16* >(mData + 2)','nativeContentReadU16(mData + 2)')
$a=$s.IndexOf('void* BinaryDataContentAccessor::getPointer(')
if($a -lt 0){throw 'Content accessor lookup anchor missing'}
$lookup=@"
void* BinaryDataContentAccessor::getPointer(const char* name, u8* data) const {
    if (!name || !data) return nullptr;
    const u16 hash = u16(MR::getHashCode(name));
    for (s32 i = 0; i < getAttributeNum(); ++i) {
        const u8* entry = mData + 4 + i * 4;
        if (nativeContentReadU16(entry) == hash) {
            const u16 offset = nativeContentReadU16(entry + 2);
            return offset < getDataSize() ? data + offset : nullptr;
        }
    }
    return nullptr;
}
"@
$s=$s.Substring(0,$a)+$lookup
$prefix=@"
#include <stdexcept>
namespace {
u16 nativeContentReadU16(const u8* data) { return u16((u16(data[0]) << 8) | data[1]); }
void nativeContentWriteU16(JSUMemoryOutputStream& stream, u16 value) {
    const u8 bytes[2] = {u8(value >> 8), u8(value)};
    stream.write(bytes, 2);
}
}
"@
# Helpers require the original stream/type declarations.
$s=$s.Replace('#include "Game/System/BinaryDataContentAccessor.hpp"','#include "Game/System/BinaryDataContentAccessor.hpp"'+[Environment]::NewLine+$prefix)
$s=$s.Replace('    u16 hash = MR::getHashCode(pName);','    if (!pName || mStream.getAvailable() < 4 || attributeSize > 65535 - mDataSize || mAttributeNum >= 65535) throw std::length_error("Save content attribute capacity");'+[Environment]::NewLine+'    u16 hash = MR::getHashCode(pName);')
$s=$s.Replace('    u16 numAttributes = 0;','    if (!pData || dataSize < 4) throw std::invalid_argument("Save content header capacity");'+[Environment]::NewLine+'    u16 numAttributes = 0;')
Set-Content "$n\src\generated\BinaryDataContentAccessor.cpp" $s
$memory=Get-Content "$p\Petari\src\JSystem\JSupport\JSUMemoryStream.cpp" -Raw
$memory=$memory.Replace('memcpy(pDest, (void*)((int)mBuffer + mPosition), length);','memcpy(pDest, static_cast<const u8*>(mBuffer) + mPosition, length);')
$memory=$memory.Replace('memcpy((void*)((int)mBuffer + mPosition), pSrc, length);','memcpy(static_cast<u8*>(mBuffer) + mPosition, pSrc, length);')
Set-Content "$n\src\generated\NativeMemoryStreams.cpp" $memory

