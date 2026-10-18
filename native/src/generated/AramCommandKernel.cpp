#include <JSystem/JKernel/JKRAramPiece.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
JKRAMCommand::JKRAMCommand() : mPieceLink(this), field_0x30(this) {
    OSInitMessageQueue(&mMessageQueue, &mMessage, 1);
    mCallback = NULL;
    field_0x5C = NULL;
    field_0x60 = 0;
    field_0x8C = NULL;
    field_0x90 = NULL;
    field_0x94 = NULL;
}

JKRAMCommand::~JKRAMCommand() {
    if (field_0x8C)
        delete field_0x8C;
    if (field_0x90)
        delete field_0x90;

    if (field_0x94)
        JKRFree(field_0x94);
}

void JKRAMCommand::operator delete(void* memory) noexcept { JKRHeap::free(memory,nullptr); }