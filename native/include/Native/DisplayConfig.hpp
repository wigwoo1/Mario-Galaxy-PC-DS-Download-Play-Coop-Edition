#pragma once
#include <cstdint>
namespace Native {
// Original VI/SC compatibility settings, separate from host window resolution.
struct DisplayConfig {
    std::uint32_t televisionFormat=0;
    bool widescreen=true;
    bool progressive=true;
    bool digitalOutput=true;
    bool euRgb60=false;
};
DisplayConfig displayConfig();
void setDisplayConfig(const DisplayConfig& config);
}
