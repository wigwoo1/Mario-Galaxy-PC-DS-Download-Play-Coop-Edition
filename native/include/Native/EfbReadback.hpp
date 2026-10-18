#pragma once
#include <cstdint>
#include <stdexcept>

namespace Native {
using EfbPixelReader=std::uint32_t(*)(void*,std::uint16_t,std::uint16_t);
class EfbReadbackBinding {
public:
 EfbReadbackBinding(void* context,EfbPixelReader reader);
 ~EfbReadbackBinding();
 EfbReadbackBinding(const EfbReadbackBinding&)=delete;
 EfbReadbackBinding& operator=(const EfbReadbackBinding&)=delete;
private:void* previousContext;EfbPixelReader previousReader;
};
std::uint32_t peekEfbArgb(std::uint16_t x,std::uint16_t y);
void setEfbAlphaReadMode(unsigned mode);
unsigned efbAlphaReadMode();
}
