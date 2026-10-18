#pragma once
#include <Native/D3dEfb.hpp>
#include <Native/D3dEfbRegionClear.hpp>
#include <Native/GxCommandState.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Progressive Wii XFB copy path. XFB is packed YUYV 4:2:2; this is distinct
// from the RGB565 texture-copy helper used by GX texture captures.
class NativeD3dXfbCopy {
    Native::D3dEfb& efb;
    Native::GpuResources& resources;
    ID3D11DeviceContext& context;

    static std::uint8_t clampByte(int value) {
        return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
    }
    static std::array<int, 3> toYuv(const std::array<int, 3>& rgb) {
        const auto r = rgb[0], g = rgb[1], b = rgb[2];
        return {((66 * r + 129 * g + 25 * b + 128) >> 8) + 16,
                ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128,
                ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128};
    }
    static std::array<int, 3> filteredPixel(const std::vector<std::uint8_t>& rgba,
                                           unsigned width, unsigned height,
                                           unsigned x, unsigned y,
                                           unsigned top, unsigned sourceHeight,
                                           const std::array<unsigned, 7>& weights,
                                           unsigned weightSum) {
        std::array<unsigned long long, 3> sum{};
        for (int tap = 0; tap < 7; ++tap) {
            if (!weights[tap]) continue;
            const auto sampleY = static_cast<unsigned>(std::clamp<int>(
                static_cast<int>(top + y) + tap - 3, static_cast<int>(top),
                static_cast<int>(top + sourceHeight - 1)));
            if (sampleY >= height || x >= width)
                throw std::out_of_range("GX XFB source rectangle exceeds the EFB");
            const auto at = (std::size_t(sampleY) * width + x) * 4;
            for (unsigned c = 0; c < 3; ++c)
                sum[c] += std::uint64_t(rgba[at + c]) * weights[tap];
        }
        std::array<int, 3> result{};
        for (unsigned c = 0; c < 3; ++c)
            result[c] = static_cast<int>((sum[c] + weightSum / 2) / weightSum);
        return result;
    }

public:
    NativeD3dXfbCopy(Native::D3dEfb& target, Native::GpuResources& storage,
                     ID3D11DeviceContext& gpu)
        : efb(target), resources(storage), context(gpu) {}

    void operator()(const Native::GxCommandState::CopyOperation& operation) const {
        if (!operation.display())
            throw std::invalid_argument("Native XFB handler received a texture copy");
        if (operation.verticalScale() || operation.fieldMode() != 0 || operation.gamma() != 0)
            throw std::invalid_argument("Only progressive, unscaled, gamma-1.0 XFB copies are supported");

        const unsigned width = operation.width(), height = operation.height();
        const unsigned left = operation.left(), top = operation.top();
        const unsigned stride = operation.strideBytes();
        if (!width || !height || (width & 1) || stride < width * 2 || (stride & 31))
            throw std::invalid_argument("Invalid GX XFB dimensions or stride");
        if (std::size_t(stride) > SIZE_MAX / height)
            throw std::overflow_error("GX XFB allocation size overflow");
        if (left > efb.getWidth() || width > efb.getWidth() - left ||
            top > efb.getHeight() || height > efb.getHeight() - top)
            throw std::out_of_range("GX XFB source rectangle exceeds the EFB");

        std::array<unsigned, 7> weights{};
        const auto filter0 = operation.registers[0x53];
        const auto filter1 = operation.registers[0x54];
        unsigned weightSum = 0;
        for (unsigned i = 0; i < 7; ++i) {
            weights[i] = ((i < 4 ? filter0 : filter1) >> ((i < 4 ? i : i - 4) * 6)) & 63;
            weightSum += weights[i];
        }
        if (!weightSum) { weights[3] = 64; weightSum = 64; }
        if (weightSum != 64)
            throw std::invalid_argument("GX XFB vertical filter weights must total 64");

        const auto rgba = efb.readColor();
        const auto* previousXfb = resources.resolve(operation.destinationAddress(), std::size_t(stride) * height);
        std::vector<std::uint8_t> xfb(previousXfb, previousXfb + std::size_t(stride) * height);
        for (unsigned y = 0; y < height; ++y) {
            auto* row = xfb.data() + std::size_t(y) * stride;
            for (unsigned x = 0; x < width; x += 2) {
                const auto yuv0 = toYuv(filteredPixel(rgba, efb.getWidth(), efb.getHeight(), left + x, y, top, height, weights, weightSum));
                const auto yuv1 = toYuv(filteredPixel(rgba, efb.getWidth(), efb.getHeight(), left + x + 1, y, top, height, weights, weightSum));
                const auto chromaU = (yuv0[1] + yuv1[1] + 1) / 2;
                const auto chromaV = (yuv0[2] + yuv1[2] + 1) / 2;
                row[x * 2 + 0] = clampByte(yuv0[0]);
                row[x * 2 + 1] = clampByte(chromaU);
                row[x * 2 + 2] = clampByte(yuv1[0]);
                row[x * 2 + 3] = clampByte(chromaV);
            }
        }
        resources.write(operation.destinationAddress(), xfb.data(), xfb.size());

        if (operation.clear()) {
            const auto ar = operation.registers[0x4f], gb = operation.registers[0x50];
            const float clearRgba[] = {float(ar & 255) / 255.f, float((gb >> 8) & 255) / 255.f,
                                        float(gb & 255) / 255.f, float((ar >> 8) & 255) / 255.f};
            const float depth = float(operation.registers[0x51] & 0xffffff) / 16777216.f;
            Native::clearD3dEfbRegion(context, efb, left, top, width, height, clearRgba, depth);
        }
    }
};
