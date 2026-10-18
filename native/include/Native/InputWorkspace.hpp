#pragma once
#include <stdint.h>
// Configure before creating the original heap hierarchy; not a controller backend.
bool nativeConfigureInputWorkspace(uint32_t bytes);
uint32_t nativeInputWorkspaceBytes();
