#pragma once
#include <revolution/gd/GDBase.h>
#include <vector>
namespace Native {
// Background jobs own their GX command bytes. Private J3D list scopes nest
// inside this writer; completed jobs publish commands before signaling ready.
class AsyncGxCommands {
    GDLObj* previous;
    GDLObj writer{};
    std::vector<u8> storage;
    bool active;
public:
    AsyncGxCommands();
    ~AsyncGxCommands();
    void commit();
    AsyncGxCommands(const AsyncGxCommands&)=delete;
    AsyncGxCommands& operator=(const AsyncGxCommands&)=delete;
};
// Called by the render thread before it emits the next frame's draw commands.
void drainAsyncGxCommands();
}
