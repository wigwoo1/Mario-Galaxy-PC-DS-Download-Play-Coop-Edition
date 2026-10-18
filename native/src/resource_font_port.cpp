#include <Native/GpuResources.hpp>
#include "native_font_records.hpp"
#include <nw4r/ut/ResFont.h>
#include <Native/FontResource.hpp>
#include <mutex>
namespace {
struct GpuFontRecords:NativeFontRecords {
 explicit GpuFontRecords(const std::vector<uint8_t>& bytes):NativeFontRecords(bytes){Native::gpuResources().add(glyph.sheetImage,size_t(glyph.sheetSize)*glyph.sheetNum);}
 ~GpuFontRecords(){Native::gpuResources().remove(glyph.sheetImage);}
};
std::mutex fontMutex;std::map<const void*,std::vector<uint8_t>> resources;auto& fontOwners(){Native::gpuResources();static std::map<nw4r::ut::ResFont*,std::unique_ptr<GpuFontRecords>> owned;return owned;}}
bool nativeRegisterFontResource(const void* buffer,size_t bytes){
    if(!buffer || bytes<16 || bytes>64*1024*1024)return false;
    auto begin=static_cast<const uint8_t*>(buffer);std::vector<uint8_t> copy(begin,begin+bytes);
    std::lock_guard<std::mutex> lock(fontMutex);return resources.emplace(buffer,std::move(copy)).second;
}
void nativeUnregisterFontResource(const void* buffer){std::lock_guard<std::mutex> lock(fontMutex);resources.erase(buffer);}
namespace nw4r {namespace ut {
ResFont::ResFont(){}
ResFont::~ResFont(){RemoveResource();}
bool ResFont::SetResource(void* buffer){
    std::lock_guard<std::mutex> lock(fontMutex);if(!IsManaging(nullptr))return false;
    auto found=resources.find(buffer);if(found==resources.end())return false;
    std::unique_ptr<GpuFontRecords> records;
    try{records=std::make_unique<GpuFontRecords>(found->second);}catch(const std::exception&){return false;}
    auto info=&records->info;fontOwners().emplace(this,std::move(records));SetResourceBuffer(buffer,info);InitReaderFunc(GetEncoding());return true;
}
void ResFont::RemoveResource(){std::lock_guard<std::mutex> lock(fontMutex);RemoveResourceBuffer();fontOwners().erase(this);}
}}


