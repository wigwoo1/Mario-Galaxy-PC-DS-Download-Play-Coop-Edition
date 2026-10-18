#include <Native/DspMailbox.hpp>
#include <revolution/dsp.h>

#include <cassert>
#include <cstdint>
#include <cstdio>

namespace {
struct TestBackend { unsigned interrupts = 0; u32 lastWord = 0; };

void consumeAndReply(void* context) {
    auto* backend = static_cast<TestBackend*>(context);
    ++backend->interrupts;
    auto& mailbox = Native::nativeDspMailbox();
    assert(mailbox.tryReadToDsp(&backend->lastWord));
    assert(mailbox.tryWriteFromDsp(backend->lastWord ^ 0xA5A55A5Au));
}
}

int main() {
    auto& mailbox = Native::nativeDspMailbox();
    DSPInit();
    assert(DSPCheckMailToDSP() == 0 && DSPCheckMailFromDSP() == 0);
    assert(DSPReadMailFromDSP() == 0);

    DSPSendMailToDSP(0x12345678u);
    assert(DSPCheckMailToDSP() == 1);
    DSPSendMailToDSP(0xDEADBEEFu);
    assert(DSPCheckMailToDSP() == 1);
    assert(mailbox.protocolErrors() == 1);

    DSPAssertInt();
    assert(DSPCheckMailToDSP() == 1); // no HLE backend: never fabricate an ack
    assert(DSPCheckMailFromDSP() == 0);

    TestBackend backend{};
    nativeSetDspMailboxInterrupt(consumeAndReply, &backend);
    DSPAssertInt();
    assert(backend.interrupts == 1 && backend.lastWord == 0x12345678u);
    assert(DSPCheckMailToDSP() == 0 && DSPCheckMailFromDSP() == 1);
    assert(DSPReadMailFromDSP() == (0x12345678u ^ 0xA5A55A5Au));
    assert(DSPCheckMailFromDSP() == 0);
    assert(DSPReadMailFromDSP() == 0);

    assert(mailbox.tryWriteFromDsp(0xCAFEBABEu));
    assert(DSPCheckMailFromDSP() == 1);
    assert(!mailbox.tryWriteFromDsp(0x01020304u)); // preserve unread response
    assert(mailbox.protocolErrors() == 2);
    assert(DSPReadMailFromDSP() == 0xCAFEBABEu);
    assert(DSPCheckMailFromDSP() == 0);

    nativeSetDspMailboxInterrupt(nullptr, nullptr);
    DSPInit();
    assert(DSPCheckMailToDSP() == 0 && DSPCheckMailFromDSP() == 0);
    std::puts("PASS native DSP mailbox single-word/busy/read/reply semantics (test backend only; no DSP execution)");
}
