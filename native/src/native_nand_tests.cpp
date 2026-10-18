#include "native_nand_port.hpp"
#include <revolution/nand.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <condition_variable>
#include <mutex>
#include <chrono>
extern "C" s32 NANDReadAsync(NANDFileInfo*,void*,u32,NANDCallback,NANDCommandBlock*);
extern "C" s32 NANDWriteAsync(NANDFileInfo*,const void*,u32,NANDCallback,NANDCommandBlock*);
extern "C" s32 NANDGetLengthAsync(NANDFileInfo*,u32*,NANDCallback,NANDCommandBlock*);
extern "C" s32 NANDPrivateSafeOpenAsync(const char*,NANDFileInfo*,u8,void*,u32,NANDCallback,NANDCommandBlock*);
extern "C" s32 NANDSafeCloseAsync(NANDFileInfo*,NANDCallback,NANDCommandBlock*);
extern "C" s32 NANDSeek(NANDFileInfo*,s32,s32);
extern "C" void NANDSetUserData(NANDCommandBlock*,void*);
extern "C" void* NANDGetUserData(const NANDCommandBlock*);
namespace { std::mutex callbackMutex;std::condition_variable callbackCv;bool callbackReady=false;s32 callbackResult=NAND_RESULT_UNKNOWN;void completed(s32 result,NANDCommandBlock*) {std::lock_guard<std::mutex> guard(callbackMutex);callbackResult=result;callbackReady=true;callbackCv.notify_one();} void awaitCallback(){std::unique_lock<std::mutex> guard(callbackMutex);assert(callbackCv.wait_for(guard,std::chrono::seconds(5),[]{return callbackReady;}));callbackReady=false;} }
int main(){auto root=std::filesystem::current_path()/"build"/"nand-io-fixture";Native::setNandRoot(root);NANDFileInfo info{};
auto firstRunRoot=std::filesystem::temp_directory_path()/("galaxy-nand-first-read-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
assert(!std::filesystem::exists(firstRunRoot));Native::setNandRoot(firstRunRoot);
assert(NANDOpen("GameData.bin",&info,NAND_ACCESS_READ)==NAND_RESULT_NOEXISTS);
Native::setNandRoot(root);
assert(NANDCreate("../escape",0,0)==NAND_RESULT_INVALID);
auto created=NANDCreate("roundtrip.dat",0,0);assert(created==0||created==NAND_RESULT_EXISTS);assert(NANDCreate("roundtrip.dat",0,0)==NAND_RESULT_EXISTS);
assert(NANDOpen("missing.dat",&info,NAND_ACCESS_READ)==NAND_RESULT_NOEXISTS);
assert(NANDOpen("roundtrip.dat",&info,NAND_ACCESS_WRITE)==0);const char payload[]="Galaxy native save fixture";assert(NANDWrite(&info,payload,sizeof(payload))==sizeof(payload));assert(NANDRead(&info,&created,1)==NAND_RESULT_ACCESS);assert(NANDClose(&info)==0);assert(NANDClose(&info)==NAND_RESULT_INVALID);
assert(NANDOpen("roundtrip.dat",&info,NAND_ACCESS_READ)==0);u32 length=0;assert(NANDGetLength(&info,&length)==0&&length==sizeof(payload));char output[64]{};assert(NANDRead(&info,output,sizeof(output))==sizeof(payload));assert(!std::memcmp(output,payload,sizeof(payload)));assert(NANDRead(&info,output,1)==0);assert(NANDWrite(&info,payload,1)==NAND_RESULT_ACCESS);assert(NANDClose(&info)==0);
u32 capacity=99;assert(NANDCheck(0,0,&capacity)==0&&capacity==0);assert(NANDCheck(0x401,0,&capacity)==0&&(capacity&NAND_CHECK_HOME_INSSPACE));assert(NANDCheck(0,34,&capacity)==0&&(capacity&NAND_CHECK_HOME_INSINODE));assert(NANDCheck(0,0,nullptr)==NAND_RESULT_INVALID);
std::filesystem::create_directories(root/"moved");
assert(NANDOpen("roundtrip.dat",&info,NAND_ACCESS_READ)==0);assert(NANDDelete("roundtrip.dat")==NAND_RESULT_OPENFD);assert(NANDMove("roundtrip.dat","moved")==NAND_RESULT_OPENFD);assert(NANDClose(&info)==0);
assert(NANDMove("roundtrip.dat","moved")==0);assert(NANDOpen("roundtrip.dat",&info,NAND_ACCESS_READ)==NAND_RESULT_NOEXISTS);assert(NANDOpen("moved/roundtrip.dat",&info,NAND_ACCESS_READ)==0);std::memset(output,0,sizeof(output));assert(NANDRead(&info,output,sizeof(output))==sizeof(payload)&&!std::memcmp(output,payload,sizeof(payload)));assert(NANDClose(&info)==0);
assert(NANDDelete("moved")==NAND_RESULT_NOTEMPTY);char home[NAND_MAX_PATH]{};assert(NANDGetHomeDir(home)==0&&!std::strcmp(home,"/"));assert(NANDGetHomeDir(nullptr)==NAND_RESULT_INVALID);assert(NANDCreate("roundtrip.dat",0,0)==0);NANDFileInfo old{};assert(NANDOpen("roundtrip.dat",&old,NAND_ACCESS_WRITE)==0);const char stale[]="old";assert(NANDWrite(&old,stale,sizeof(stale))==sizeof(stale));assert(NANDClose(&old)==0);
assert(NANDOpen("roundtrip.dat",&old,NAND_ACCESS_READ)==0);assert(NANDMove("moved/roundtrip.dat",home)==NAND_RESULT_OPENFD);assert(NANDClose(&old)==0);
assert(NANDMove("moved/roundtrip.dat",home)==0);assert(NANDOpen("roundtrip.dat",&old,NAND_ACCESS_READ)==0);assert(NANDGetLength(&old,&length)==0&&length==sizeof(payload));std::memset(output,0,sizeof(output));assert(NANDRead(&old,output,sizeof(output))==sizeof(payload)&&!std::memcmp(output,payload,sizeof(payload)));assert(NANDClose(&old)==0);assert(NANDDelete("roundtrip.dat")==0);assert(NANDDelete("moved/roundtrip.dat")==NAND_RESULT_NOEXISTS);assert(NANDDelete("moved")==0);assert(NANDMove("../escape","moved")==NAND_RESULT_INVALID);
NANDCommandBlock block{};int userData=42;NANDSetUserData(&block,&userData);assert(NANDGetUserData(&block)==&userData);assert(NANDPrivateSafeOpenAsync("async.dat",&info,NAND_ACCESS_RW,nullptr,0,completed,&block)==NAND_RESULT_OK);awaitCallback();assert(callbackResult==NAND_RESULT_OK&&info.mark==5);const char asyncPayload[]="callback I/O";assert(NANDWriteAsync(&info,asyncPayload,sizeof(asyncPayload),completed,&block)==NAND_RESULT_OK);awaitCallback();assert(callbackResult==sizeof(asyncPayload));assert(NANDSeek(&info,0,SEEK_SET)==NAND_RESULT_OK);char asyncOutput[32]{};assert(NANDReadAsync(&info,asyncOutput,sizeof(asyncOutput),completed,&block)==NAND_RESULT_OK);awaitCallback();assert(callbackResult==sizeof(asyncPayload)&&!std::memcmp(asyncOutput,asyncPayload,sizeof(asyncPayload)));u32 asyncLength=0;assert(NANDGetLengthAsync(&info,&asyncLength,completed,&block)==NAND_RESULT_OK);awaitCallback();assert(callbackResult==NAND_RESULT_OK&&asyncLength==sizeof(asyncPayload));assert(NANDSafeCloseAsync(&info,completed,&block)==NAND_RESULT_OK);awaitCallback();assert(callbackResult==NAND_RESULT_OK);assert(NANDDelete("async.dat")==NAND_RESULT_OK);
std::puts("PASS native NAND disk round trip, async callback I/O, safe-open/close, length, EOF, missing file, access modes, move/delete and capacity flags");}



