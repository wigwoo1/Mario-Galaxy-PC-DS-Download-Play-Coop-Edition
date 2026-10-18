#include <cstring>
#include <stdint.h>
extern "C" void DCZeroRange(void* memory,uint32_t bytes){if(bytes)std::memset(memory,0,bytes);}
