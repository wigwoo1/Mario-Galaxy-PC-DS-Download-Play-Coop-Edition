#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#include <array>
#include <vector>
#include <stdexcept>
#include <cstdint>

inline void cryptoCheck(NTSTATUS status) {
    if (status < 0) throw std::runtime_error("Windows cryptography operation failed");
}
class AesCbc {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_KEY_HANDLE key = nullptr;
public:
    explicit AesCbc(const uint8_t* bytes) {
        cryptoCheck(BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_AES_ALGORITHM, nullptr, 0));
        try {
            cryptoCheck(BCryptSetProperty(algorithm, BCRYPT_CHAINING_MODE,
                reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_CBC)), sizeof(BCRYPT_CHAIN_MODE_CBC), 0));
            cryptoCheck(BCryptGenerateSymmetricKey(algorithm, &key, nullptr, 0, const_cast<PUCHAR>(bytes), 16, 0));
        } catch (...) { BCryptCloseAlgorithmProvider(algorithm, 0); throw; }
    }
    AesCbc(const AesCbc&) = delete;
    AesCbc& operator=(const AesCbc&) = delete;
    ~AesCbc() { if (key) BCryptDestroyKey(key); if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); }
    std::vector<uint8_t> decrypt(const uint8_t* source, size_t size, const uint8_t* iv) {
        if (size % 16 || size > 0xffffffffu) throw std::runtime_error("Invalid AES block size");
        std::array<uint8_t, 16> workingIv{};
        std::copy_n(iv, 16, workingIv.begin());
        std::vector<uint8_t> result(size);
        ULONG written = 0;
        cryptoCheck(BCryptDecrypt(key, const_cast<PUCHAR>(source), static_cast<ULONG>(size), nullptr,
            workingIv.data(), 16, result.data(), static_cast<ULONG>(size), &written, 0));
        if (written != size) throw std::runtime_error("AES output size mismatch");
        return result;
    }
};
inline std::array<uint8_t, 20> sha1(const uint8_t* bytes, size_t size) {
    if (size > 0xffffffffu) throw std::runtime_error("SHA1 input too large");
    std::array<uint8_t, 20> result{};
    cryptoCheck(BCryptHash(BCRYPT_SHA1_ALG_HANDLE, nullptr, 0, const_cast<PUCHAR>(bytes),
        static_cast<ULONG>(size), result.data(), 20));
    return result;
}
