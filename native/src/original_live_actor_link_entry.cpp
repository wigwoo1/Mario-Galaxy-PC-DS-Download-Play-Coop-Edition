#include <Game/LiveActor/LiveActor.hpp>
// Link-only integration entry. Do not execute before original actor scene services are installed.
extern "C" __declspec(dllexport) void* galaxy_actor_link_probe(){return new LiveActor("native actor integration probe");}
