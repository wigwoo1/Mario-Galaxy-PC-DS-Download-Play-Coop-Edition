#pragma once
#include <cstdint>
namespace Native {
// Call on the render thread only after the token's preceding GPU work completes.
void dispatchDrawSyncCallback(std::uint16_t token);
}
