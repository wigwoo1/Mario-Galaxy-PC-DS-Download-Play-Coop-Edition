#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRDvdAramRipper.hpp>
#include <JSystem/JKernel/JKRDvdFile.hpp>
JKRAramBlock* JKRDvdAramRipper::loadToAram(s32 entryNumber, u32 address, JKRExpandSwitch expandSwitch, u32 param_3, u32 param_4, u32* param_5) {
    JKRDvdFile dvdFile;
    if (!dvdFile.open(entryNumber)) {
        return nullptr;
    } else {
        return loadToAram(&dvdFile, address, expandSwitch, param_3, param_4, param_5);
    }
}

JKRAramBlock* JKRDvdAramRipper::loadToAram(JKRDvdFile* dvdFile, u32 address, JKRExpandSwitch expandSwitch, u32 param_3, u32 param_4, u32* param_5) {
    JKRADCommand* command = loadToAram_Async(dvdFile, address, expandSwitch, nullptr, param_3, param_4, param_5);
    if (!command) return nullptr;
    syncAram(command, 0);

    if (command->field_0x48 < 0) {
        command->~JKRADCommand(); JKRHeap::free(command, nullptr);
        return nullptr;
    }

    if (address) {
        command->~JKRADCommand(); JKRHeap::free(command, nullptr);
        return reinterpret_cast< JKRAramBlock* >(-1);
    }

    JKRAramBlock* result = command->mBlock;
    command->~JKRADCommand(); JKRHeap::free(command, nullptr);
    return result;
}


