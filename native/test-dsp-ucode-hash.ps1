$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$sourcePath = Join-Path $p 'Petari\src\JSystem\JAudio2\dsptask.cpp'
$source = Get-Content -LiteralPath $sourcePath -Raw
$match = [regex]::Match($source, 'static\s+u16\s+jdsp\[\].*?=\s*\{(.*?)\};', [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (!$match.Success) { throw 'Could not locate Petari jdsp microcode array.' }

$body = [regex]::Replace($match.Groups[1].Value, '/\*.*?\*/|//[^\r\n]*', '', [System.Text.RegularExpressions.RegexOptions]::Singleline)
$tokens = [regex]::Matches($body, '0x[0-9a-fA-F]+|\b\d+\b')
if ($tokens.Count -eq 0) { throw 'The Petari jdsp array contains no instruction words.' }

# The DSP task is stored as big-endian 16-bit words in guest memory. Dolphin's
# HashEctor is a one-byte XOR followed by a 32-bit rotate-left by three.
[long]$hash = 0
$byteCount = 0
foreach ($token in $tokens) {
    if ($token.Value.StartsWith('0x')) {
        $word = [Convert]::ToUInt16($token.Value.Substring(2), 16)
    } else {
        $word = [Convert]::ToUInt16($token.Value, 10)
    }
    foreach ($byte in @([byte]($word -shr 8), [byte]($word -band 0xff))) {
        $hash = (($hash -bxor [long]$byte) -band 4294967295)
        $hash = ((($hash -shl 3) -bor ($hash -shr 29)) -band 4294967295)
        $byteCount += 1
    }
}

if ($byteCount -ne 0x1f00) { throw "Unexpected jdsp microcode byte size: 0x$('{0:X}' -f $byteCount)." }
$expectedHash = [Convert]::ToInt64('D643001F', 16)
if ($hash -ne $expectedHash) { throw "Unexpected jdsp HashEctor: 0x$('{0:X8}' -f $hash)." }

$dolphinZelda = Join-Path $p 'external\dolphin-dsp-reference\Source\Core\Core\HW\DSPHLE\UCodes\Zelda.cpp'
if (Test-Path -LiteralPath $dolphinZelda) {
    $hleSource = Get-Content -LiteralPath $dolphinZelda -Raw
    if ($hleSource -notmatch '\{\s*0xD643001F,\s*NO_ARAM\s*\|\s*MAKE_DOLBY_LOUDER\s*\|\s*COMBINED_CMD_0D\s*\}') {
        throw 'The pinned DSP reference no longer identifies this hash as the Super Mario Galaxy Zelda UCode.'
    }
    Write-Output 'Reference match: Dolphin maps 0xD643001F to its Zelda UCode with the SMG flags.'
}

Write-Output ("PASS Petari jdsp HashEctor 0x{0:X8}, {1} bytes, big-endian DSP word layout" -f $hash, $byteCount)
