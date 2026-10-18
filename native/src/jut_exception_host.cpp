#include <JSystem/JUtility/JUTException.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
#include <JSystem/JUtility/JUTConsole.hpp>
#include <Native/HeapPlacement.hpp>
#include <chrono>
#include <thread>
#include <windows.h>

JUTException* JUTException::sErrorManager = nullptr;
JUTExceptionUserCallback JUTException::sPreUserCallback = nullptr;
void* JUTException::sConsoleBuffer = nullptr;
u32 JUTException::sConsoleBufferSize = 0;
JUTConsole* JUTException::sConsole = nullptr;

JUTException::~JUTException() = default;
void* JUTException::run() { return nullptr; }

JUTException::JUTException(JUTDirectPrint* directPrint) : JKRThread(0x4000, 16, 0) {
    mDirectPrint = directPrint;
    mGamePad = nullptr;
    mGamePadPort = JUTGamePad::Port_Unknown;
    mPrintWaitTime0 = 10;
    mPrintWaitTime1 = 10;
    mTraceSuppress = ~u32(0);
    field_0x98 = 0;
    mPrintFlags = 0x1F;
}

JUTException* JUTException::create(JUTDirectPrint* directPrint) {
    if (sErrorManager == nullptr) {
        sErrorManager = new (JKRHeap::sSystemHeap, 0) JUTException(directPrint);
    }
    return sErrorManager;
}

JUTExceptionUserCallback JUTException::setPreUserCallback(JUTExceptionUserCallback callback) {
    JUTExceptionUserCallback previous = sPreUserCallback;
    sPreUserCallback = callback;
    return previous;
}

void JUTException::waitTime(s32 durationMilliseconds) {
    if (durationMilliseconds > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMilliseconds));
    }
}

bool JUTException::readPad(u32* trigger, u32* hold) {
    if (!trigger || !hold) {
        return false;
    }
    *trigger = 0;
    *hold = 0;
    const auto down = [](int key) { return (GetAsyncKeyState(key) & 0x8000) != 0; };
    const auto edge = [](int key) { return (GetAsyncKeyState(key) & 1) != 0; };
    const struct { int key; u32 mask; } keys[] = {
        {VK_UP, 0x0008}, {VK_DOWN, 0x0004}, {VK_LEFT, 0x0001}, {VK_RIGHT, 0x0002},
        {VK_RETURN, 0x0100}, {VK_SPACE, 0x0010},
    };
    for (const auto& item : keys) {
        if (down(item.key)) *hold |= item.mask;
        if (edge(item.key)) *trigger |= item.mask;
    }
    return *trigger != 0;
}

void JUTException::createConsole(void* consoleBuffer, u32 consoleBufferSize) {
    if (!consoleBuffer || consoleBufferSize == 0) return;
    const u32 lines = JUTConsole::getLineFromObjectSize(consoleBufferSize, 0x32);
    if (lines == 0) return;
    sConsoleBuffer = consoleBuffer;
    sConsoleBufferSize = consoleBufferSize;
    sConsole = JUTConsole::create(0x32, sConsoleBuffer, sConsoleBufferSize);
    JUTConsoleManager::getManager()->setDirectConsole(sConsole);
    sConsole->mFontSizeX = 10.0f;
    sConsole->mFontSizeY = 6.0f;
    sConsole->mPositionX = 15;
    sConsole->mPositionY = 26;
    sConsole->mHeight = 23;
    if (sConsole->mHeight > sConsole->mMaxLines) sConsole->mHeight = sConsole->mMaxLines;
    sConsole->mVisible = true;
    sConsole->mOutput = JUTConsole::OUTPUT_OSREPORT | JUTConsole::OUTPUT_CONSOLE;
}
