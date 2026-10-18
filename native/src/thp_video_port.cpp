#include <Native/ThpVideo.hpp>
#include <revolution/thp.h>
#include <windows.h>
#include <wincodec.h>
#include <objbase.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
#include <vector>

namespace NativeThp {
namespace {
struct FrameContext {
    const u8* bytes;
    std::size_t byteCount;
    u32 width;
    u32 height;
};

thread_local const FrameContext* activeContext = nullptr;
std::atomic<bool> decoderInitialized{false};

template <typename T> class ComOwner {
public:
    ComOwner() noexcept : mValue(nullptr) {}
    ~ComOwner() { reset(); }
    ComOwner(const ComOwner&) = delete;
    ComOwner& operator=(const ComOwner&) = delete;
    T** put() noexcept { reset(); return &mValue; }
    T* get() const noexcept { return mValue; }
    T* operator->() const noexcept { return mValue; }
    void reset() noexcept { if (mValue) { mValue->Release(); mValue = nullptr; } }
private:
    T* mValue;
};

class ComApartment {
public:
    ComApartment() noexcept : mShouldUninitialize(false), mReady(false) {
        const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        mShouldUninitialize = SUCCEEDED(result);
        mReady = SUCCEEDED(result) || result == RPC_E_CHANGED_MODE;
    }
    ~ComApartment() { if (mShouldUninitialize) CoUninitialize(); }
    bool ready() const noexcept { return mReady; }
private:
    bool mShouldUninitialize;
    bool mReady;
};

bool decodeJpeg(const FrameContext& context, std::vector<u8>& bgra) {
    if (!context.bytes || !context.byteCount || context.byteCount > 32u * 1024u * 1024u ||
        !context.width || !context.height || (context.width & 1u) || (context.height & 1u) ||
        context.width > 8192 || context.height > 8192 ||
        std::uint64_t(context.width) * context.height > 32u * 1024u * 1024u) return false;

    ComApartment apartment;
    if (!apartment.ready()) return false;
    ComOwner<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(factory.put()));
    if (FAILED(hr)) return false;
    ComOwner<IWICStream> stream;
    if (FAILED(factory->CreateStream(stream.put()))) return false;
    if (FAILED(stream->InitializeFromMemory(const_cast<BYTE*>(context.bytes), DWORD(context.byteCount)))) return false;
    ComOwner<IWICBitmapDecoder> decoder;
    if (FAILED(factory->CreateDecoderFromStream(stream.get(), nullptr, WICDecodeMetadataCacheOnLoad, decoder.put()))) return false;
    ComOwner<IWICBitmapFrameDecode> frame;
    if (FAILED(decoder->GetFrame(0, frame.put()))) return false;
    UINT width = 0, height = 0;
    if (FAILED(frame->GetSize(&width, &height)) || width != context.width || height != context.height) return false;

    ComOwner<IWICFormatConverter> converter;
    if (FAILED(factory->CreateFormatConverter(converter.put()))) return false;
    if (FAILED(converter->Initialize(frame.get(), GUID_WICPixelFormat32bppBGRA,
                                     WICBitmapDitherTypeNone, nullptr, 0.0,
                                     WICBitmapPaletteTypeCustom))) return false;
    const std::uint64_t stride64 = std::uint64_t(width) * 4;
    const std::uint64_t bytes64 = stride64 * height;
    if (stride64 > UINT32_MAX || bytes64 > UINT32_MAX) return false;
    try { bgra.resize(static_cast<std::size_t>(bytes64)); }
    catch (const std::bad_alloc&) { return false; }
    return SUCCEEDED(converter->CopyPixels(nullptr, UINT(stride64), UINT(bytes64), bgra.data()));
}

u8 clampByte(double value) noexcept {
    return static_cast<u8>(std::clamp<long>(std::lround(value), 0, 255));
}

bool decodePlanar(const FrameContext& context, u8* yPlane, u8* uPlane, u8* vPlane) {
    std::vector<u8> bgra;
    if (!decodeJpeg(context, bgra)) return false;
    const std::size_t width = context.width;
    const std::size_t height = context.height;
    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            const u8* pixel = bgra.data() + (y * width + x) * 4;
            const double b = pixel[0], g = pixel[1], r = pixel[2];
            yPlane[y * width + x] = clampByte(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }
    for (std::size_t y = 0; y < height; y += 2) {
        for (std::size_t x = 0; x < width; x += 2) {
            double u = 0.0, v = 0.0;
            for (std::size_t dy = 0; dy < 2; ++dy) {
                for (std::size_t dx = 0; dx < 2; ++dx) {
                    const u8* pixel = bgra.data() + ((y + dy) * width + x + dx) * 4;
                    const double b = pixel[0], g = pixel[1], r = pixel[2];
                    u += 128.0 - 0.168736 * r - 0.331264 * g + 0.5 * b;
                    v += 128.0 + 0.5 * r - 0.418688 * g - 0.081312 * b;
                }
            }
            const std::size_t chromaIndex = (y / 2) * (width / 2) + (x / 2);
            uPlane[chromaIndex] = clampByte(u * 0.25);
            vPlane[chromaIndex] = clampByte(v * 0.25);
        }
    }
    return true;
}
} // namespace

VideoFrameScope::VideoFrameScope(const void* bytes, std::size_t byteCount, u32 width, u32 height) noexcept
    : mPrevious(activeContext), mContext(mStorage) {
    static_assert(sizeof(FrameContext) <= sizeof(mStorage));
    const FrameContext context{static_cast<const u8*>(bytes), byteCount, width, height};
    std::memcpy(mStorage, &context, sizeof(context));
    activeContext = static_cast<const FrameContext*>(mContext);
}

VideoFrameScope::~VideoFrameScope() {
    activeContext = static_cast<const FrameContext*>(mPrevious);
}

bool videoDecoderInitialized() noexcept { return decoderInitialized.load(std::memory_order_acquire); }
} // namespace NativeThp

extern "C" BOOL THPInit(void) {
    NativeThp::decoderInitialized.store(true, std::memory_order_release);
    return TRUE;
}

extern "C" s32 THPVideoDecode(void* file, void* tileY, void* tileU, void* tileV, void* work) {
    if (!file) return 25;
    if (!tileY || !tileU || !tileV) return 27;
    if (!work) return 26;
    if (!NativeThp::videoDecoderInitialized()) return 29;
    const auto* context = static_cast<const NativeThp::FrameContext*>(NativeThp::activeContext);
    if (!context || context->bytes != file) return 3;
    try {
        return NativeThp::decodePlanar(*context, static_cast<u8*>(tileY), static_cast<u8*>(tileU),
                                      static_cast<u8*>(tileV)) ? 0 : 1;
    } catch (const std::bad_alloc&) {
        return 6;
    } catch (...) {
        return 1;
    }
}
