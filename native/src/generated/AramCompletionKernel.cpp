#include <JSystem/JKernel/JKRAramPiece.hpp>
#include <JSystem/JKernel/JKRDecomp.hpp>
void JKRAramPiece::doneDMA(std::uintptr_t requestAddress) {
    JKRAMCommand* command = reinterpret_cast< JKRAMCommand* >(requestAddress);

    if (command->field_0x60 != 0) {
        if (command->field_0x60 == 2) {
            JKRDecompress_SendCommand(command->mDecompCommand);
        }
    } else if (command->mCallback) {
        (*command->mCallback)(requestAddress);
    } else if (command->field_0x5C) {
        OSSendMessage(command->field_0x5C, command, OS_MESSAGE_NOBLOCK);
    } else {
        OSSendMessage(&command->mMessageQueue, command, OS_MESSAGE_NOBLOCK);
    }
}