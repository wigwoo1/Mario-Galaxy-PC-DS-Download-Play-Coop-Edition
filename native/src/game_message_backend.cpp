#include <Native/GameMessages.hpp>
#include <stdexcept>
namespace {NativeGameMessageBackend* backend=nullptr;}
void nativeBindGameMessageBackend(NativeGameMessageBackend* value){backend=value;}
void nativeUnbindGameMessageBackend(NativeGameMessageBackend* value){if(backend==value)backend=nullptr;}
void nativeInitGameMessages(MessageHolder& holder,bool system){if(!backend)throw std::runtime_error("Native message backend not bound");backend->load(holder,system);}
#include <Game/System/MessageHolder.hpp>
namespace { MessageHolder* sceneHolder=nullptr; }
void nativeBindSceneMessages(MessageHolder& holder){sceneHolder=&holder;}
void nativeUnbindSceneMessages(MessageHolder& holder){if(sceneHolder==&holder)sceneHolder=nullptr;}
MessageData* nativeSceneMessages(){if(!sceneHolder || !sceneHolder->mSceneMessageData)throw std::runtime_error("Native scene messages are not initialized");return sceneHolder->mSceneMessageData;}
