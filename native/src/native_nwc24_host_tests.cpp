#include <Game/NWC24/NWC24System.hpp>
#include <Game/NWC24/NWC24Function.hpp>
#include <Game/NWC24/ReceiverTagMail.hpp>
#include <cassert>
#include <cstdio>

int main() {
    NWC24System system(nullptr, 0);
    NWC24Err error = NWC24_OK;
    s32 errorCode = 0;
    assert(!system.open(&error, &errorCode));
    assert(error == NWC24_ERR_DISABLED && errorCode == static_cast<s32>(NWC24_ERR_DISABLED));
    assert(!system.send(nullptr, nullptr, nullptr, 0, nullptr, 0, 0, false, 0));
    u32 sentSize = 99;
    assert(system.isSent(&error, &sentSize));
    assert(error == NWC24_ERR_DISABLED && sentSize == 0);
    assert(system.close(&error) && error == NWC24_OK);

    const u16 title[] = {'M','a','i','l',0};
    const u16 body[] = {'H','i',0};
    assert(MR::calcWiiMailSize(title, body, 0, 0) > 360);
    assert(!MR::checkWiiMailLimit(1));
    MR::updateWiiMailSentSize(1024);
    const wchar_t* sender = MR::getMailSender("WiiMessageFromKinopio");
    assert(sender && sender[0] == L'\0');

    ReceiverTagMail receiver("task", "message", "sender");
    assert(receiver.isDone() && receiver.isError());
    const u8 picture[] = {0, 1, 2};
    receiver.send(picture, sizeof(picture), true);
    receiver.update();
    assert(receiver.isDone() && receiver.isError());
    std::puts("PASS NWC24 host-disabled state and explicit no-send behavior");
}
