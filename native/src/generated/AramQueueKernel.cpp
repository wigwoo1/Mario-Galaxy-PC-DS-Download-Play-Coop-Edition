#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRAramPiece.hpp>
#include <JSystem/JUtility/JUTException.hpp>
#include <Native/HeapPlacement.hpp>
JSUList<JKRAMCommand> JKRAramPiece::sAramPieceCommandList;
JKRAMCommand* JKRAramPiece::prepareCommand(int direction, u32 src, u32 dst, u32 length, JKRAramBlock* block, JKRAMCommand::AsyncCallback callback) {
    JKRAMCommand* command = new (JKRGetSystemHeap(), -4) JKRAMCommand();
    command->mTransferDirection = direction;
    command->mSrc = src;
    command->mDst = dst;
    command->mAramBlock = block;
    command->mDataLength = length;
    command->mCallback = callback;
    return command;
}

JKRAMCommand* JKRAramPiece::orderAsync(int direction, u32 source, u32 destination, u32 length, JKRAramBlock* block,
                                       JKRAMCommand::AsyncCallback callback) {
    lock();
    if ((source & 0x1f) != 0 || (destination & 0x1f) != 0) {
        OSReport("direction = %x\n", direction);
        OSReport("source = %x\n", source);
        OSReport("destination = %x\n", destination);
        OSReport("length = %x\n", length);
        JUTException::panic(__FILE__, 108, "illegal address. abort.");
    }

    JKRAramCommand* message = new (JKRGetSystemHeap(), -4) JKRAramCommand();
    JKRAMCommand* command = JKRAramPiece::prepareCommand(direction, source, destination, length, block, callback);
    message->setting(1, command);

    OSSendMessage(&JKRAram::sMessageQueue, message, OS_MESSAGE_BLOCK);
    if (command->mCallback != NULL) {
        sAramPieceCommandList.append(&command->mPieceLink);
    }

    unlock();
    return command;
}

BOOL JKRAramPiece::sync(JKRAMCommand* command, int is_non_blocking) {
    OSMessage message;

    lock();
    if (is_non_blocking == 0) {
        OSReceiveMessage(&command->mMessageQueue, &message, OS_MESSAGE_BLOCK);
        sAramPieceCommandList.remove(&command->mPieceLink);
        unlock();
        return 1;
    }

    if (!OSReceiveMessage(&command->mMessageQueue, &message, OS_MESSAGE_NOBLOCK)) {
        unlock();
        return 0;
    }

    sAramPieceCommandList.remove(&command->mPieceLink);
    unlock();
    return 1;
}

BOOL JKRAramPiece::orderSync(int direction, u32 source, u32 destination, u32 length, JKRAramBlock* block) {
    lock();

    JKRAMCommand* command = JKRAramPiece::orderAsync(direction, source, destination, length, block, NULL);
    BOOL result = JKRAramPiece::sync(command, 0);
    delete command;

    unlock();
    return result;
}

