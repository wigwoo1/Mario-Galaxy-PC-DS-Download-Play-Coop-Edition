#include "native_nand_port.hpp"
#include "native_nand_path.hpp"
#include <revolution/nand.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <climits>
#include <map>
#include <mutex>
#include <functional>
#include <thread>
#define NOMINMAX
#include <windows.h>
namespace {
std::mutex lock;
std::filesystem::path root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path()/"saves";
struct Handle { FILE* file; u8 access; NANDFileInfo* owner; std::filesystem::path path; };
std::map<s32,Handle> handles;
s32 nextId=1;
s32 fileError() { switch(errno) {case ENOENT:return NAND_RESULT_NOEXISTS;case EACCES:return NAND_RESULT_ACCESS;case ENOSPC:return NAND_RESULT_MAXBLOCKS;case EMFILE:return NAND_RESULT_MAXFD;default:return NAND_RESULT_UNKNOWN;} }
Handle* lookup(NANDFileInfo* info) {if(!info)return nullptr;auto i=handles.find(info->fileDescriptor);return i!=handles.end()&&i->second.owner==info?&i->second:nullptr;}
s32 dispatchAsync(NANDCallback callback,NANDCommandBlock* block,std::function<s32()> operation) {
 if(!callback||!block)return NAND_RESULT_INVALID;
 block->callback=reinterpret_cast<void*>(callback);
 try {
  std::thread([callback,block,operation=std::move(operation)]() mutable {
   s32 result=NAND_RESULT_UNKNOWN;
   try { result=operation(); } catch(...) { result=NAND_RESULT_UNKNOWN; }
   callback(result,block);
  }).detach();
 } catch(...) { return NAND_RESULT_ALLOC_FAILED; }
 return NAND_RESULT_OK;
}
}
void Native::setNandRoot(const std::filesystem::path& path) {std::lock_guard<std::mutex> guard(lock);if(!handles.empty())throw std::logic_error("NAND files still open");root=std::filesystem::absolute(path);}
extern "C" s32 NANDCreate(const char* name,u8 permission,u8 attribute) {
 std::lock_guard<std::mutex> guard(lock);
 try {auto path=Native::nandFilePath(root,name);std::filesystem::create_directories(path.parent_path());if(std::filesystem::exists(path))return NAND_RESULT_EXISTS;
 FILE* file=_wfopen(path.c_str(),L"wbx");if(!file)return fileError();return std::fclose(file)==0?NAND_RESULT_OK:fileError();
 }catch(const std::invalid_argument&){return NAND_RESULT_INVALID;}catch(const std::filesystem::filesystem_error&){return NAND_RESULT_ACCESS;}
}
extern "C" s32 NANDOpen(const char* name,NANDFileInfo* info,u8 access) {
 std::lock_guard<std::mutex> guard(lock);if(!info||access<1||access>3)return NAND_RESULT_INVALID;
 for(const auto& entry:handles)if(entry.second.owner==info)return NAND_RESULT_OPENFD;
 try {
#ifdef GALAXY_BOOT_TRACE
 std::fprintf(stderr,"NAND open: name=%s root=%ls access=%u\n",name?name:"<null>",root.c_str(),unsigned(access));
#endif
 auto path=Native::nandFilePath(root,name);FILE* file=_wfopen(path.c_str(),access==NAND_ACCESS_READ?L"rb":L"r+b");if(!file){
 const int savedError=errno;
#ifdef GALAXY_BOOT_TRACE
 std::fprintf(stderr,"NAND open failed: path=%ls errno=%d win32=%lu\n",path.c_str(),savedError,GetLastError());
#endif
 errno=savedError;const s32 result=fileError();
#ifdef GALAXY_BOOT_TRACE
 std::fprintf(stderr,"NAND open return=%d\n",int(result));
#endif
 return result;}
 if(nextId==INT_MAX){std::fclose(file);return NAND_RESULT_MAXFD;}s32 id=nextId++;handles.emplace(id,Handle{file,access,info,path});*info={};info->fileDescriptor=id;info->accType=access;return NAND_RESULT_OK;
 }catch(const std::invalid_argument&){return NAND_RESULT_INVALID;}catch(const std::filesystem::filesystem_error& error){
#ifdef GALAXY_BOOT_TRACE
 std::fprintf(stderr,"NAND path failed: code=%d detail=%s\n",error.code().value(),error.what());
#endif
 return NAND_RESULT_ACCESS;}
}
extern "C" s32 NANDClose(NANDFileInfo* info) {std::lock_guard<std::mutex> guard(lock);auto h=lookup(info);if(!h)return NAND_RESULT_INVALID;auto file=h->file;handles.erase(info->fileDescriptor);info->fileDescriptor=-1;return std::fclose(file)==0?NAND_RESULT_OK:fileError();}
extern "C" s32 NANDRead(NANDFileInfo* info,void* buffer,u32 size) {std::lock_guard<std::mutex> guard(lock);auto h=lookup(info);if(!h||(!buffer&&size)||size>INT_MAX)return NAND_RESULT_INVALID;if(!(h->access&NAND_ACCESS_READ))return NAND_RESULT_ACCESS;auto count=std::fread(buffer,1,size,h->file);return std::ferror(h->file)?fileError():s32(count);}
extern "C" s32 NANDWrite(NANDFileInfo* info,const void* buffer,u32 size) {std::lock_guard<std::mutex> guard(lock);auto h=lookup(info);if(!h||(!buffer&&size)||size>INT_MAX)return NAND_RESULT_INVALID;if(!(h->access&NAND_ACCESS_WRITE))return NAND_RESULT_ACCESS;auto count=std::fwrite(buffer,1,size,h->file);return count!=size?fileError():s32(count);}
extern "C" s32 NANDGetLength(NANDFileInfo* info,u32* length) {std::lock_guard<std::mutex> guard(lock);auto h=lookup(info);if(!h||!length)return NAND_RESULT_INVALID;auto pos=_ftelli64(h->file);if(pos<0||_fseeki64(h->file,0,SEEK_END))return fileError();auto end=_ftelli64(h->file);if(_fseeki64(h->file,pos,SEEK_SET))return fileError();if(end<0||end>UINT_MAX)return NAND_RESULT_INVALID;*length=u32(end);return NAND_RESULT_OK;}

extern "C" s32 NANDDelete(const char* name) {
 std::lock_guard<std::mutex> guard(lock);
 try {auto path=Native::nandFilePath(root,name);for(const auto& entry:handles)if(entry.second.path==path)return NAND_RESULT_OPENFD;
 if(!std::filesystem::exists(path))return NAND_RESULT_NOEXISTS;
 if(std::filesystem::is_directory(path)&&!std::filesystem::is_empty(path))return NAND_RESULT_NOTEMPTY;
 return std::filesystem::remove(path)?NAND_RESULT_OK:NAND_RESULT_NOEXISTS;
 }catch(const std::invalid_argument&){return NAND_RESULT_INVALID;}catch(const std::filesystem::filesystem_error&){return NAND_RESULT_ACCESS;}
}
extern "C" s32 NANDMove(const char* name,const char* directory) {
 std::lock_guard<std::mutex> guard(lock);
 try {auto source=Native::nandFilePath(root,name);auto dir=(directory&&std::strcmp(directory,"/")==0)?std::filesystem::weakly_canonical(root):Native::nandFilePath(root,directory);auto dest=dir/source.filename();
 for(const auto& entry:handles)if(entry.second.path==source||entry.second.path==dest)return NAND_RESULT_OPENFD;
 if(!std::filesystem::exists(source)||!std::filesystem::is_directory(dir))return NAND_RESULT_NOEXISTS;
 if(source==dest)return NAND_RESULT_OK;
 if(std::filesystem::is_regular_file(source)) {
 if(std::filesystem::exists(dest)&&!std::filesystem::is_regular_file(dest))return NAND_RESULT_ACCESS;
 if(MoveFileExW(source.c_str(),dest.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))return NAND_RESULT_OK;
 switch(GetLastError()){case ERROR_FILE_NOT_FOUND:case ERROR_PATH_NOT_FOUND:return NAND_RESULT_NOEXISTS;case ERROR_DISK_FULL:return NAND_RESULT_MAXBLOCKS;case ERROR_SHARING_VIOLATION:return NAND_RESULT_OPENFD;default:return NAND_RESULT_ACCESS;}
 }
 if(std::filesystem::exists(dest))return NAND_RESULT_EXISTS;
 std::filesystem::rename(source,dest);return NAND_RESULT_OK;
 }catch(const std::invalid_argument&){return NAND_RESULT_INVALID;}catch(const std::filesystem::filesystem_error&){return NAND_RESULT_ACCESS;}
}
extern "C" s32 NANDCheck(u32 requestedBlocks,u32 requestedNodes,u32* answer) {
 std::lock_guard<std::mutex> guard(lock);if(!answer)return NAND_RESULT_INVALID;
 try {std::filesystem::create_directories(root);uint64_t blocks=0,nodes=0;
 for(const auto& entry:std::filesystem::recursive_directory_iterator(root)) {if(entry.is_symlink())return NAND_RESULT_ACCESS;++nodes;if(entry.is_regular_file())blocks+=(entry.file_size()+16383)/16384;}
 const uint64_t required=uint64_t(requestedBlocks)*16384;u32 flags=0;
 if(blocks+requestedBlocks>0x400)flags|=NAND_CHECK_HOME_INSSPACE;
 if(nodes+requestedNodes>0x21)flags|=NAND_CHECK_HOME_INSINODE;
 if(std::filesystem::space(root).available<required)flags|=NAND_CHECK_SYS_INSSPACE;
 *answer=flags;return NAND_RESULT_OK;
 }catch(const std::filesystem::filesystem_error& error){
#ifdef GALAXY_BOOT_TRACE
 std::fprintf(stderr,"NAND capacity check failed: root=%ls code=%d detail=%s\n",root.c_str(),error.code().value(),error.what());
#endif
 return NAND_RESULT_ACCESS;}
}

extern "C" s32 NANDGetHomeDir(char* directory) {if(!directory)return NAND_RESULT_INVALID;std::strcpy(directory,"/");return NAND_RESULT_OK;}

// Host compatibility for Petari callers that use the SDK's private aliases and
// callback-style file APIs. The host implementation completes on a worker
// thread; caller-owned blocks, buffers and output fields must remain alive until
// their callback runs, as with the Wii async contract.
extern "C" s32 NANDPrivateOpen(const char* name,NANDFileInfo* info,u8 access) { return NANDOpen(name,info,access); }
extern "C" s32 NANDPrivateCreate(const char* name,u8 permission,u8 attribute) { return NANDCreate(name,permission,attribute); }
extern "C" s32 NANDSeek(NANDFileInfo* info,s32 offset,s32 whence) {
 std::lock_guard<std::mutex> guard(lock);auto h=lookup(info);if(!h||whence<SEEK_SET||whence>SEEK_END)return NAND_RESULT_INVALID;
 return _fseeki64(h->file,offset,whence)==0?NAND_RESULT_OK:fileError();
}
extern "C" void NANDSetUserData(NANDCommandBlock* block,void* data) { if(block)block->userData=data; }
extern "C" void* NANDGetUserData(const NANDCommandBlock* block) { return block?block->userData:nullptr; }

extern "C" s32 NANDReadAsync(NANDFileInfo* info,void* buffer,u32 size,NANDCallback cb,NANDCommandBlock* block) {
 return dispatchAsync(cb,block,[=]{return NANDRead(info,buffer,size);});
}
extern "C" s32 NANDWriteAsync(NANDFileInfo* info,const void* buffer,u32 size,NANDCallback cb,NANDCommandBlock* block) {
 return dispatchAsync(cb,block,[=]{return NANDWrite(info,buffer,size);});
}
extern "C" s32 NANDSeekAsync(NANDFileInfo* info,s32 offset,s32 whence,NANDCallback cb,NANDCommandBlock* block) {
 return dispatchAsync(cb,block,[=]{return NANDSeek(info,offset,whence);});
}
extern "C" s32 NANDGetLengthAsync(NANDFileInfo* info,u32* length,NANDCallback cb,NANDCommandBlock* block) {
 if(block)block->length=length;
 return dispatchAsync(cb,block,[=]{return NANDGetLength(info,length);});
}
extern "C" s32 NANDPrivateDeleteAsync(const char* name,NANDCallback cb,NANDCommandBlock* block) {
 const std::string path=name?name:"";return dispatchAsync(cb,block,[path]{return NANDDelete(path.c_str());});
}
extern "C" s32 NANDPrivateCreateAsync(const char* name,u8 permission,u8 attribute,NANDCallback cb,NANDCommandBlock* block) {
 const std::string path=name?name:"";return dispatchAsync(cb,block,[path,permission,attribute]{return NANDPrivateCreate(path.c_str(),permission,attribute);});
}
extern "C" s32 NANDPrivateCreateDirAsync(const char* name,u8,u8,NANDCallback cb,NANDCommandBlock* block) {
 const std::string path=name?name:"";return dispatchAsync(cb,block,[path]{
  try {std::lock_guard<std::mutex> guard(lock);auto resolved=Native::nandFilePath(root,path.c_str());if(std::filesystem::exists(resolved))return NAND_RESULT_EXISTS;std::filesystem::create_directories(resolved.parent_path());return std::filesystem::create_directory(resolved)?NAND_RESULT_OK:NAND_RESULT_ACCESS;}
  catch(const std::invalid_argument&){return NAND_RESULT_INVALID;}catch(const std::filesystem::filesystem_error&){return NAND_RESULT_ACCESS;}
 });
}
extern "C" s32 NANDPrivateSafeOpenAsync(const char* name,NANDFileInfo* info,u8 access,void*,u32,NANDCallback cb,NANDCommandBlock* block) {
 const std::string path=name?name:"";return dispatchAsync(cb,block,[path,info,access]{
  if(!info)return s32(NAND_RESULT_INVALID);
  s32 result=NANDPrivateOpen(path.c_str(),info,access);
  if(result==NAND_RESULT_NOEXISTS&&access!=NAND_ACCESS_READ){result=NANDPrivateCreate(path.c_str(),NAND_PERM_RWALL,0);if(result==NAND_RESULT_EXISTS)result=NAND_RESULT_OK;if(result==NAND_RESULT_OK)result=NANDPrivateOpen(path.c_str(),info,access);}
  if(result==NAND_RESULT_OK){info->stage=2;info->mark=access==NAND_ACCESS_READ?3:5;}
  return result;
 });
}
extern "C" s32 NANDSafeCloseAsync(NANDFileInfo* info,NANDCallback cb,NANDCommandBlock* block) {
 return dispatchAsync(cb,block,[=]{if(!info||(info->mark!=3&&info->mark!=5))return s32(NAND_RESULT_INVALID);return NANDClose(info);});
}


