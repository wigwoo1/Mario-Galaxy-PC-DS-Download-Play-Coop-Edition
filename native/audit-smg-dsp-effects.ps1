$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$reference=Join-Path $n 'build\dolphin-dsp-reference'
$core=Join-Path $reference 'Source\Core'
$out=Join-Path $n 'build\smg-dsp-effects-audit'
$headers=Join-Path $out 'headers'
foreach($dir in @('Core\DSP','Common\Logging','fmt')){New-Item -ItemType Directory -Force -Path (Join-Path $headers $dir) | Out-Null}
$inputs=@('Core\DSP\DSPDisassembler.cpp','Core\DSP\DSPDisassembler.h','Core\DSP\DSPTables.cpp','Core\DSP\DSPTables.h','Core\DSP\DSPCommon.h','Core\DSP\DSPCore.h','Common\CommonTypes.h')
$hashes=@(foreach($rel in $inputs){$source=Join-Path $core $rel;if(!(Test-Path -LiteralPath $source)){throw "Missing local reference: $rel"};[pscustomobject]@{Path=$rel;SHA256=(Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash}})
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $out 'reference-inputs.json') -Encoding utf8
foreach($rel in @('Core\DSP\DSPDisassembler.h','Core\DSP\DSPTables.h','Core\DSP\DSPCommon.h','Common\CommonTypes.h')){Copy-Item -LiteralPath (Join-Path $core $rel) -Destination (Join-Path $headers $rel)}
# Decoder/opcode tables remain original. Only trim DSPCore's unrelated runtime
# dependencies down to its complete register enum; replace logging/ASCII printing.
$source=[IO.File]::ReadAllText((Join-Path $core 'Core\DSP\DSPCore.h'))
$registers=@([regex]::Matches($source,'enum : int\s*\{.*?\};',[Text.RegularExpressions.RegexOptions]::Singleline) | Where-Object{$_.Value.Contains('DSP_REG_AR0')-or $_.Value.Contains('DSP_REG_ACC0_FULL')} | ForEach-Object{$_.Value})
if($registers.Count-ne 2){throw 'Original DSP physical/combined register enums missing'}
[IO.File]::WriteAllText((Join-Path $headers 'Core\DSP\DSPCore.h'),"#pragma once`n#include <Common/CommonTypes.h>`nnamespace DSP {`n"+($registers-join "`n")+"`n}`n")
[IO.File]::WriteAllText((Join-Path $headers 'fmt\format.h'),"#pragma once`n#include <format>`nnamespace fmt {using std::format;}`n")
[IO.File]::WriteAllText((Join-Path $headers 'Common\Logging\Log.h'),"#pragma once`n#include <stdexcept>`ntemplate<class... T>[[noreturn]] inline void AuditDecodeError(T&&...){throw std::runtime_error(`"DSP disassembly diagnostic error`");}`n#define ERROR_LOG_FMT(category,...) AuditDecodeError(__VA_ARGS__)`n")
[IO.File]::WriteAllText((Join-Path $headers 'Common\StringUtil.h'),"#pragma once`n#include <string>`n#include <cctype>`nnamespace Common {inline void ToLower(std::string* s){for(char& c:*s)c=char(std::tolower(static_cast<unsigned char>(c)));}}`n")
$driver=@'
#include <Native/DspUcodeImage.hpp>
#include <Core/DSP/DSPDisassembler.h>
#include <fstream>
#include <cstdio>
#include <vector>
int main(int argc,char** argv){
 if(argc!=2)return 1;
 const auto size=Native::originalDspUcodeImageBytes();const auto* data=Native::originalDspUcodeImage();
 if(size!=0x1F00)return 2;
 std::vector<u16> words(size/2);for(unsigned i=0;i<words.size();++i)words[i]=(u16(data[i*2])<<8)|data[i*2+1];
 DSP::AssemblerSettings settings;settings.show_pc=true;settings.show_hex=true;settings.lower_case_ops=false;
 DSP::DSPDisassembler disassembler(settings);std::string text;
 if(!disassembler.Disassemble(words,text))return 3;
 std::ofstream output(argv[1],std::ios::binary);output<<text;if(!output)return 4;
 std::printf("PASS original Dolphin instruction decoder:0x%zx SMG uCode bytes disassembled; printing/logging adapters only\n",size);
}
'@
$driverPath=Join-Path $out 'disassemble-smg.cpp';[IO.File]::WriteAllText($driverPath,$driver)
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$exe=Join-Path $out 'smg-disassemble.exe';$log=Join-Path $out 'build.txt'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++23 -O2 -DFMT_HEADER_ONLY -target x86_64-windows-gnu "-I$headers" "-I$n\include" "-I$reference\Externals\fmt\fmt\include" $driverPath (Join-Path $core 'Core\DSP\DSPDisassembler.cpp') (Join-Path $core 'Core\DSP\DSPTables.cpp') "$n\src\generated\DspUcodeImage.cpp" -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 25;exit 1}
& (Join-Path $n 'test-dsp-ucode-hash.ps1') *> (Join-Path $out 'image-hash.txt')
& $exe (Join-Path $out 'smg-jdsp.disasm.txt') *> (Join-Path $out 'run.txt')
$code=$LASTEXITCODE;Get-Content -LiteralPath (Join-Path $out 'run.txt');exit $code
