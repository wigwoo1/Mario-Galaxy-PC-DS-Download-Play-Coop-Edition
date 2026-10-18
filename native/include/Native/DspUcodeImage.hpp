#pragma once

#include <cstddef>
#include <cstdint>

namespace Native {
const std::uint8_t* originalDspUcodeImage() noexcept;
std::size_t originalDspUcodeImageBytes() noexcept;
}
