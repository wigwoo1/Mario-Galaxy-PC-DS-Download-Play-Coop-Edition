#include <Game/NWC24/NWC24System.hpp>
#include <Game/NWC24/NWC24Function.hpp>
#include <Game/NWC24/ReceiverTagMail.hpp>
#include <revolution/nwc24.h>

namespace {
constexpr NWC24Err kHostMailUnsupported = NWC24_ERR_DISABLED;
std::size_t utf16Length(const u16* text) {
    if (!text) return 0;
    std::size_t size = 0;
    while (text[size]) ++size;
    return size;
}
}

// Wii Message Board is not available to a Windows host. Keep the game-facing
// API deterministic and explicit: no host method reports a successful send.
NWC24System::NWC24System(JKRHeap*, s32)
    : _0(false), mWorkBuffer(nullptr), mVFWorkBuffer(nullptr), mSendThread(nullptr) {}

bool NWC24System::open(NWC24Err* error, s32* errorCode) {
    _0 = false;
    if (error) *error = kHostMailUnsupported;
    if (errorCode) *errorCode = static_cast<s32>(kHostMailUnsupported);
    return false;
}

bool NWC24System::close(NWC24Err* error) {
    _0 = false;
    if (error) *error = NWC24_OK;
    return true;
}

bool NWC24System::send(const u16*, const u16*, const u8*, u32, const u8*, u32, u16, bool, u8) {
    return false;
}

bool NWC24System::isSent(NWC24Err* error, u32* size) {
    if (error) *error = kHostMailUnsupported;
    if (size) *size = 0;
    return true;
}

namespace MR {
u32 calcWiiMailSize(const u16* title, const u16* body, u32 pictureSize, u32 letterSize) {
    const u32 titleUnits = static_cast<u32>(utf16Length(title) + 1);
    const u32 bodyUnits = static_cast<u32>(utf16Length(body) + 1);
    const auto encodedSize=[](u32 bytes) { return (bytes * 4 + 2) / 3 + bytes / 57 * 2 + 4; };
    return 360 + encodedSize(titleUnits * 2) + encodedSize(bodyUnits * 2) +
           encodedSize(pictureSize) + encodedSize(letterSize);
}
bool checkWiiMailLimit(u32) { return false; }
void updateWiiMailSentSize(u32) {}
const wchar_t* getMailSender(const char*) {
    static constexpr wchar_t emptySender[] = L"";
    return emptySender;
}
}

ReceiverTagMail::ReceiverTagMail(const char* taskName, const char* messageID, const char* senderID)
    : mTaskName(taskName), mMessageID(messageID), mSenderID(senderID), mMessage(nullptr),
      mMessageLength(0), mIsError(true), _18(1) {}

void ReceiverTagMail::send(const u8*, u32, bool) {
    mIsError = true;
    _18 = 1;
}
void ReceiverTagMail::update() {}
bool ReceiverTagMail::isDone() const { return true; }
bool ReceiverTagMail::isError() const { return true; }
