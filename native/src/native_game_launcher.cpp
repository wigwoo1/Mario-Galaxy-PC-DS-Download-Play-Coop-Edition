#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <string>
#include <string>

using StartGame = int (*)(const wchar_t*);
using GameError = const char* (*)();

int main() {
    int argumentCount = 0;
    LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
    if (!arguments || argumentCount < 1) {
        MessageBoxW(nullptr, L"Could not read the launch arguments.", L"Native Galaxy", MB_OK | MB_ICONERROR);
        return 1;
    }

    const std::filesystem::path executable(arguments[0]);
    const auto hostLibrary = executable.parent_path() / L"galaxy-native.dll";
    const auto discImage = argumentCount > 1
        ? std::filesystem::path(arguments[1])
        : executable.parent_path() / L"Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs";
    LocalFree(arguments);

    if (!std::filesystem::is_regular_file(discImage)) {
        const auto message = L"The WBFS image could not be found:\n" + discImage.wstring();
        MessageBoxW(nullptr, message.c_str(), L"Native Galaxy", MB_OK | MB_ICONERROR);
        return 2;
    }

    HMODULE game = LoadLibraryW(hostLibrary.c_str());
    if (!game) {
        MessageBoxW(nullptr, (L"Could not load the game runtime:\n" + hostLibrary.wstring()).c_str(),
                    L"Native Galaxy", MB_OK | MB_ICONERROR);
        return 3;
    }

    auto start = reinterpret_cast<StartGame>(GetProcAddress(game, "galaxyNativeStartGame"));
    auto error = reinterpret_cast<GameError>(GetProcAddress(game, "galaxyNativeGameError"));
    if (!start || !error) {
        MessageBoxW(nullptr, L"The native game runtime is missing its start exports.",
                    L"Native Galaxy", MB_OK | MB_ICONERROR);
        FreeLibrary(game);
        return 4;
    }

    const int result = start(discImage.c_str());
    const char* detail = error();
    const std::string text = detail ? detail : "No diagnostic was returned.";
    const int required = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    std::wstring message(size_t(required > 0 ? required : 1), L'\0');
    if (required > 0) MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, message.data(), required);
    MessageBoxW(nullptr, message.c_str(), L"Native Galaxy could not continue", MB_OK | MB_ICONERROR);
    FreeLibrary(game);
    return result ? result : 5;
}
