#pragma once
#include <filesystem>
bool nativeBindDvd(const std::filesystem::path&);
void nativeReleaseDvd();
bool nativeDvdIsBound();
#include <vector>
#include <cstdint>
bool nativeReadDvdEntry(int,std::vector<uint8_t>&);
