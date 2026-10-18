#include "Native/GpuResources.hpp"
#include "Native/GDCompat.hpp"
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <windows.h>
#include <memory>
#include <cstdio>
#include <Native/TexturePointers.hpp>
#include <unordered_map>
#include <mutex>
namespace Native {
namespace {
struct TexturePointers {void* image=nullptr;void* palette=nullptr;bool valid=true;};
std::recursive_mutex resourceMutex;
std::unordered_map<const void*,TexturePointers> texturePointers;
TexturePointers findTexturePointers(const void* header){
 if(!header)throw std::invalid_argument("Null texture header");
 auto found=texturePointers.find(header);
 if(found!=texturePointers.end()){if(!found->second.valid)throw std::logic_error("Texture replacement source was released");return found->second;}
 const auto* bytes=static_cast<const std::uint8_t*>(header);std::uint32_t image,palette;std::uint16_t count;
 std::memcpy(&image,bytes+28,4);std::memcpy(&palette,bytes+12,4);std::memcpy(&count,bytes+10,2);
 const auto base=reinterpret_cast<std::uintptr_t>(header);
 return {reinterpret_cast<void*>(base+(image?image:32)),count?reinterpret_cast<void*>(base+palette):nullptr,true};
}
}
void* textureImage(const void* header){std::lock_guard<std::recursive_mutex> lock(resourceMutex);return findTexturePointers(header).image;}
void* texturePalette(const void* header){std::lock_guard<std::recursive_mutex> lock(resourceMutex);return findTexturePointers(header).palette;}
void copyTextureHeader(void* destination,const void* source){
 if(!destination||!source)throw std::invalid_argument("Null texture replacement");
 std::lock_guard<std::recursive_mutex> lock(resourceMutex);
 const auto pointers=findTexturePointers(source);
 // Both owners must have registered their ranges; removal invalidates aliases
 // before backing memory can be released or reused for another resource.
 gpuResources().byteAddress(destination);gpuResources().byteAddress(pointers.image);
 if(pointers.palette)gpuResources().byteAddress(pointers.palette);
 texturePointers.insert_or_assign(destination,pointers);
 std::memmove(destination,source,32);
}
void forgetTexturePointers(const void* range,std::size_t size){
 const auto begin=reinterpret_cast<std::uintptr_t>(range);
 auto inside=[&](const void* p){const auto value=reinterpret_cast<std::uintptr_t>(p);return value>=begin&&value-begin<size;};
 std::lock_guard<std::recursive_mutex> lock(resourceMutex);
 for(auto i=texturePointers.begin();i!=texturePointers.end();){if(inside(i->first))i=texturePointers.erase(i);else{if(inside(i->second.image)||inside(i->second.palette))i->second.valid=false;++i;}}
}
std::uint64_t GpuResources::generation() const{std::lock_guard<std::recursive_mutex> lock(resourceMutex);return revision;}
std::uint32_t GpuResources::addOwned(const void* data,std::size_t size,bool nativeWords,bool writable){
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    const auto host=reinterpret_cast<std::uintptr_t>(data);
    if(!data||!size||size>0x20000000||host>std::numeric_limits<std::uintptr_t>::max()-size)throw std::invalid_argument("invalid GPU resource range");
    const Range* containingBorrowed=nullptr;
    for(const auto& range:ranges)if(host<range.host+range.size()&&range.host<host+size){
        if(range.borrowed&&host>=range.host&&host-range.host<=range.size()&&size<=range.size()-(host-range.host)){containingBorrowed=&range;continue;}
        char detail[256];
        std::snprintf(detail,sizeof(detail),"overlapping GPU resource registration: request=%p size=%zu existing=%p size=%zu borrowed=%d",
                      data,size,reinterpret_cast<const void*>(range.host),range.size(),range.borrowed?1:0);
        throw std::invalid_argument(detail);
    }
    // Resource registrations may be narrower children of mapped image data.
    // Their separate address lets each owner remove and refresh its own copy.
    if(containingBorrowed&&writable&&!containingBorrowed->writable)throw std::invalid_argument("GPU resource range is read-only");
    const auto aligned=(std::uint64_t(size)+31)&~std::uint64_t(31);
    if(next+aligned>0x20000000)throw std::overflow_error("GPU address space exhausted");
    Range range;range.host=host;range.address=std::uint32_t(next);
    const auto* bytes=static_cast<const std::uint8_t*>(data);range.bytes.assign(bytes,bytes+size);
    range.nativeWords=nativeWords;range.writable=writable;
    ranges.push_back(std::move(range));next+=aligned;++revision;return ranges.back().address;
}
std::uint32_t GpuResources::add(const void* data,std::size_t size){return addOwned(data,size,false,false);}
std::uint32_t GpuResources::addBorrowed(const void* data,std::size_t size,std::shared_ptr<const void> lifetime,bool writable){
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    const auto host=reinterpret_cast<std::uintptr_t>(data);
    if(!data||!size||!lifetime||size>0x20000000||host>std::numeric_limits<std::uintptr_t>::max()-size)
        throw std::invalid_argument("invalid borrowed GPU resource range");
    for(const auto& range:ranges)if(host<range.host+range.size()&&range.host<host+size)
        throw std::invalid_argument("overlapping borrowed GPU resource registration");
    const auto aligned=(std::uint64_t(size)+31)&~std::uint64_t(31);
    if(next+aligned>0x20000000)throw std::overflow_error("GPU address space exhausted");
    Range range;range.host=host;range.address=std::uint32_t(next);range.borrowed=static_cast<const std::uint8_t*>(data);
    range.borrowedSize=size;range.lifetime=std::move(lifetime);range.writable=writable;
    ranges.push_back(std::move(range));next+=aligned;++revision;return ranges.back().address;
}
std::uint32_t GpuResources::addWritable(void* data,std::size_t size){
    return addOwned(data,size,false,true);
}
void GpuResources::write(std::uint32_t address,const void* source,std::size_t size){
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    if(!source&&size)throw std::invalid_argument("null GPU copy source");
    for(auto& range:ranges)if(address>=range.address){
        const auto offset=std::uint64_t(address)-range.address;
        if(offset<range.size()&&size<=range.size()-offset){
            if(!range.writable)throw std::invalid_argument("GPU destination is not writable");
            // Capture first so aliasing either live storage or its snapshot is safe.
            const auto* bytes=static_cast<const std::uint8_t*>(source);
            std::vector<std::uint8_t> captured;if(size)captured.assign(bytes,bytes+size);
            if(size){
                std::memcpy(reinterpret_cast<void*>(range.host+offset),captured.data(),size);
                // Borrowed mappings expose live storage and have no owned snapshot.
                if(!range.borrowed)std::memcpy(range.bytes.data()+offset,captured.data(),size);
                ++revision;
            }
            return;
        }
    }
    throw std::out_of_range("GPU copy destination extent");
}
std::uint32_t GpuResources::addNativeWords(const void* data,std::size_t size){
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    if(size%4)throw std::invalid_argument("native GPU word range must contain complete words");
    const auto address=addOwned(data,size,true,false);refresh(data,size);return address;
}
void GpuResources::remove(const void* data){
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    const auto host=reinterpret_cast<std::uintptr_t>(data);
    auto borrowed=ranges.end();
    for(auto i=ranges.begin();i!=ranges.end();++i)if(i->host==host){if(i->borrowed){borrowed=i;continue;}if(this==&gpuResources())forgetTexturePointers(data,i->size());ranges.erase(i);++revision;return;}
    if(borrowed!=ranges.end()){if(this==&gpuResources())forgetTexturePointers(data,borrowed->size());ranges.erase(borrowed);++revision;return;}
    throw std::invalid_argument("GPU resource is not registered");
}
void GpuResources::refresh(const void* data,std::size_t size){
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    if(!size)return;
    const auto host=reinterpret_cast<std::uintptr_t>(data);
    if(!data||host>std::numeric_limits<std::uintptr_t>::max()-size)
        throw std::invalid_argument("invalid GPU refresh range");
    const auto end=host+size;bool updated=false;
    for(auto& range:ranges){
        const auto begin=std::max(host,range.host);
        const auto finish=std::min(end,range.host+range.size());
        if(begin<finish){updated=true;
            if(range.borrowed)continue;
            if(!range.nativeWords)std::memcpy(range.bytes.data()+(begin-range.host),reinterpret_cast<const void*>(begin),finish-begin);
            else{size_t first=(begin-range.host)&~size_t(3),last=((finish-range.host)+3)&~size_t(3);
                for(size_t offset=first;offset<last;offset+=4){std::uint32_t word;std::memcpy(&word,reinterpret_cast<const void*>(range.host+offset),4);
                    for(unsigned byte=0;byte<4;++byte)range.bytes[offset+byte]=std::uint8_t(word>>(24-byte*8));}}
        }
    }
    if(updated)++revision;
}
std::uint32_t GpuResources::byteAddress(const void* data) const{
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    const auto host=reinterpret_cast<std::uintptr_t>(data);
    const Range* best=nullptr;
    for(const auto& range:ranges)if(host>=range.host&&host-range.host<range.size()&&(!best||range.size()<=best->size()))best=&range;
    if(best)return best->address+std::uint32_t(host-best->host);
    char detail[160];
    std::snprintf(detail,sizeof(detail),"GPU texture resource is not registered: host=%p",data);
    throw std::invalid_argument(detail);
}
std::uint32_t GpuResources::address(const void* data) const{
    const auto result=byteAddress(data);
    if(result%32)throw std::invalid_argument("GPU texture address is not aligned");
    return result;
}
const std::uint8_t* GpuResources::resolve(std::uint32_t address,std::size_t size) const{
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    for(const auto& range:ranges)if(address>=range.address){
        const auto offset=std::uint64_t(address)-range.address;
        if(offset<range.size()&&size<=range.size()-offset)return range.data()+offset;
    }
    throw std::out_of_range("GPU command references an invalid resource extent");
}
const std::uint8_t* GpuResources::hostAddress(std::uint32_t address,std::size_t size) const{
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    for(const auto& range:ranges)if(address>=range.address){
        const auto offset=std::uint64_t(address)-range.address;
        if(offset<range.size()&&size<=range.size()-offset)
            return reinterpret_cast<const std::uint8_t*>(range.host+offset);
    }
    throw std::out_of_range("GPU getter references an invalid host resource extent");
}
GpuResources& gpuResources(){static GpuResources resources;return resources;}
void registerExecutableImageResources(const void* anchor){
    std::lock_guard<std::recursive_mutex> lock(resourceMutex);
    HMODULE module=nullptr;
    if(!anchor||!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(anchor),&module)||!module)
        throw std::runtime_error("unable to locate native game image for GPU resources");
    auto* base=reinterpret_cast<std::uint8_t*>(module);
    const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<=0)throw std::runtime_error("invalid native game PE header");
    const auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    if(nt->Signature!=IMAGE_NT_SIGNATURE||nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC)throw std::runtime_error("native game image is not PE32+");
    static const auto lifetime=std::make_shared<const std::uint8_t>(0);
    static std::vector<HMODULE> mappedModules;
    if(std::find(mappedModules.begin(),mappedModules.end(),module)!=mappedModules.end())return;
    const auto imageSize=nt->OptionalHeader.SizeOfImage;
    const auto* section=IMAGE_FIRST_SECTION(nt);
    for(unsigned i=0;i<nt->FileHeader.NumberOfSections;++i){
        const auto flags=section[i].Characteristics;
        if(!(flags&IMAGE_SCN_CNT_INITIALIZED_DATA)||!(flags&IMAGE_SCN_MEM_READ)||(flags&IMAGE_SCN_MEM_EXECUTE))continue;
        const auto offset=section[i].VirtualAddress;
        const auto size=section[i].Misc.VirtualSize?section[i].Misc.VirtualSize:section[i].SizeOfRawData;
        if(!size||offset>=imageSize||size>imageSize-offset)throw std::runtime_error("invalid initialized-data section extent");
        gpuResources().addBorrowed(base+offset,size,lifetime);
    }
    mappedModules.push_back(module);
}
u32 gpuResourceAddress(const void* data){return gpuResources().address(data);}
}




