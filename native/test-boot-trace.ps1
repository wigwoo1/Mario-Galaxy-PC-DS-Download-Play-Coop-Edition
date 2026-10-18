$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$root=Split-Path $n -Parent
$text=[IO.File]::ReadAllText((Join-Path $n 'src\native_game_entry_host.cpp'))
$signature='void appendBootTrace(const char* step) noexcept {'
$start=$text.IndexOf($signature);if($start-lt 0){throw 'Actual boot logger function missing'}
$open=$text.IndexOf('{',$start);$depth=1;$end=$open+1;while($depth){if($text[$end]-eq '{'){$depth++}elseif($text[$end]-eq '}'){$depth--};$end++}
$actual=$text.Substring($start,$end-$start)
$harness=@"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <set>
#include <cstring>
#include <cwchar>
#include <cstdio>
static std::atomic<unsigned> opens{0};
HANDLE countedCreateFile(LPCWSTR path,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES security,DWORD creation,DWORD flags,HANDLE templateFile){++opens;return ::CreateFileW(path,access,share,security,creation,flags,templateFile);}
#define CreateFileW countedCreateFile
$actual
#undef CreateFileW
int main(){
 std::vector<std::thread> threads;
 for(unsigned t=0;t<8;++t)threads.emplace_back([t]{for(unsigned i=0;i<100;++i){char line[80];std::snprintf(line,sizeof(line),"thread=%u record=%u",t,i);appendBootTrace(line);}});
 for(auto& t:threads)t.join();
 if(opens!=1){std::fprintf(stderr,"FAIL logger opened its file %u times for 800 records; expected one retained append handle\n",opens.load());return 1;}
 wchar_t path[MAX_PATH]{};GetTempPathW(MAX_PATH,path);wcscat_s(path,L"galaxy-native-boot.log");
 HANDLE read=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(read==INVALID_HANDLE_VALUE)return 2;DWORD size=GetFileSize(read,nullptr),got=0;std::string contents(size,'\0');if(!ReadFile(read,contents.data(),size,&got,nullptr)||got!=size)return 3;CloseHandle(read);
 std::set<std::string> records;std::size_t offset=0;
 while(offset<contents.size()){auto next=contents.find("\r\n",offset);if(next==std::string::npos)return 4;auto line=contents.substr(offset,next-offset);if(!records.insert(line).second)return 5;offset=next+2;}
 if(records.size()!=800)return 6;
 for(unsigned t=0;t<8;++t)for(unsigned i=0;i<100;++i){char line[80];std::snprintf(line,sizeof(line),"thread=%u record=%u",t,i);if(!records.count(line))return 7;}
 std::puts("PASS actual boot logger: one append handle for 800 records, eight concurrent writers, intact CRLF records and live shared readback");
}
"@
$build=Join-Path $n 'build\boot-trace-regression';$testTemp=Join-Path $build 'tmp';New-Item -ItemType Directory -Force -Path $build,$testTemp|Out-Null
$source=Join-Path $build 'boot-trace-test.cpp';$exe=Join-Path $build 'boot-trace-test.exe'
[IO.File]::WriteAllText($source,$harness,[Text.UTF8Encoding]::new($false))
$oldTemp=$env:TEMP;$oldTmp=$env:TMP
try{
 $env:TEMP=$testTemp;$env:TMP=$testTemp
 $log=Join-Path $testTemp 'galaxy-native-boot.log';if(Test-Path -LiteralPath $log){Remove-Item -LiteralPath $log}
 & (Join-Path $root 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu $source -o $exe
 if($LASTEXITCODE){exit $LASTEXITCODE}
 & $exe
 exit $LASTEXITCODE
}finally{$env:TEMP=$oldTemp;$env:TMP=$oldTmp}