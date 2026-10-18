#pragma once
#include "native_aram_memory.hpp"
#include <memory>
bool nativeBindAramTransport(std::shared_ptr<NativeAramMemory>);
void nativeReleaseAramTransport();
bool nativeUploadAram(uint32_t offset,const void* data,size_t size);
bool nativeDownloadAram(uint32_t offset,void* data,size_t size);
