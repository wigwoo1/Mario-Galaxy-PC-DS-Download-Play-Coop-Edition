param([switch]$OriginalHeader)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\rfl-middle-storage-test';New-Item -ItemType Directory -Force -Path $out | Out-Null
$text=Get-Content -LiteralPath (Join-Path $p 'Petari\src\RVLFaceLib\RFL_MiddleDatabase.c') -Raw
$stop=$text.IndexOf('static BOOL checkHiddenData_');if($stop-lt 0){throw 'Middle database initializer boundary missing.'}
$text=$text.Substring(0,$stop)+@'

#include <assert.h>
#include <stdio.h>
int main(void){
 struct {u64 before;RFLMiddleDB db;u8 guard[64];} storage;
 RFLiHiddenCharData data[10];
 RFLiManager manager;memset(&manager,0,sizeof(manager));
 RFLInitMiddleDB(&manager.ctrlMgr.hiddenMDB,RFLMiddleDBType_HiddenRandom,data,10);
 memset(&storage,0xA5,sizeof(storage));memset(data,0xCC,sizeof(data));
 RFLInitMiddleDB(&storage.db,RFLMiddleDBType_HiddenRandom,data,10);
 for(unsigned i=0;i<sizeof(storage.guard);++i)if(storage.guard[i]!=0xA5){
  printf("FAIL middle database writes beyond public storage: public=%zu internal=%zu guard-byte=%u manager-working=%d\n",sizeof(RFLMiddleDB),sizeof(RFLiMiddleDB),i,manager.working);return 23;
 }
 assert(sizeof(RFLMiddleDB)>=sizeof(RFLiMiddleDB));
 assert(__alignof__(RFLMiddleDB)>=__alignof__(RFLiMiddleDB));
 assert(storage.before==0xA5A5A5A5A5A5A5A5ULL);
 assert(manager.working==0&&manager.lastErrCode==0&&manager.lastReason==0);
 RFLiMiddleDB* internal=(RFLiMiddleDB*)&storage.db;
 assert(internal->data==data&&internal->size==10&&internal->storedSize==0&&internal->callback==NULL);
 for(unsigned i=0;i<sizeof(data);++i)assert(((u8*)data)[i]==0);
 puts("PASS actual original middle database initialization: sufficient aligned host storage, intact surrounding fields, full data pointer and buffer initialization");
 return 0;
}
'@
$cpp=Join-Path $out 'middle-storage.c';[IO.File]::WriteAllText($cpp,$text)
$headers=if($OriginalHeader){Join-Path $p 'Petari\libs\RVLFaceLib\include'}else{Join-Path $n 'build\rfl-original\include'}
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$exe=Join-Path $out 'middle-storage.exe'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') cc -std=gnu99 -O2 -DTRUE=1 -DFALSE=0 -target x86_64-windows-gnu ('-I'+$headers) ('-I'+(Join-Path $n 'build\rfl-original\c-compat')) ('-I'+(Join-Path $p 'Petari\libs\RVL_SDK\include')) $cpp -o $exe *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 15;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE
