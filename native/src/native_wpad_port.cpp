#include <Native/WpadHost.hpp>
#include <array>
#include <mutex>
#include <vector>

namespace {
struct Channel {
    bool connected{};
    bool extensionPending{};
    bool connectPending{};
    WPADConnectCallback connect{};
    WPADExtensionCallback extension{};
};
struct InfoRequest { s32 channel; WPADInfo* output; WPADCallback callback; };
std::array<Channel,WPAD_MAX_CONTROLLERS> channels{};
std::vector<InfoRequest> pendingInfo;
std::mutex stateMutex;
WPADAlloc allocator{};
WPADFree deallocator{};
bool valid(s32 channel){return channel>=0&&channel<WPAD_MAX_CONTROLLERS;}
}

extern "C" void WPADInit(){
    std::lock_guard<std::mutex> lock(stateMutex);
    for(auto& channel:channels)channel=Channel{};
    channels[WPAD_CHAN0].connected=true;
    pendingInfo.clear();
}

extern "C" void WPADRegisterAllocator(WPADAlloc alloc,WPADFree freeFn){
    std::lock_guard<std::mutex> lock(stateMutex);allocator=alloc;deallocator=freeFn;
}

extern "C" void WPADDisconnect(s32 channel){
    if(!valid(channel))return;
    WPADConnectCallback callback{};bool changed=false;
    {std::lock_guard<std::mutex> lock(stateMutex);auto& state=channels[channel];changed=state.connected;state.connected=false;state.extensionPending=false;state.connectPending=false;callback=state.connect;}
    if(changed&&callback)callback(channel,WPAD_ERR_NO_CONTROLLER);
}

extern "C" s32 WPADProbe(s32 channel,u32* type){
    if(!valid(channel)){if(type)*type=WPAD_DEV_NOT_FOUND;return WPAD_ERR_INVALID;}
    std::lock_guard<std::mutex> lock(stateMutex);const bool connected=channels[channel].connected;
    if(type)*type=connected?WPAD_DEV_FREESTYLE:WPAD_DEV_NOT_FOUND;
    return connected?WPAD_ERR_NONE:WPAD_ERR_NO_CONTROLLER;
}

extern "C" WPADConnectCallback WPADSetConnectCallback(s32 channel,WPADConnectCallback callback){
    if(!valid(channel))return nullptr;std::lock_guard<std::mutex> lock(stateMutex);auto& state=channels[channel];auto prior=state.connect;state.connect=callback;state.connectPending=state.connected&&callback;return prior;
}

extern "C" WPADExtensionCallback WPADSetExtensionCallback(s32 channel,WPADExtensionCallback callback){
    if(!valid(channel))return nullptr;std::lock_guard<std::mutex> lock(stateMutex);auto& state=channels[channel];auto prior=state.extension;state.extension=callback;state.extensionPending=state.connected&&callback;return prior;
}

extern "C" void WPADSetAutoSleepTime(u8 minutes){(void)minutes;}
extern "C" u8 WPADGetSensorBarPosition(){return WPAD_SENSOR_BAR_POS_TOP;}
extern "C" void WPADControlMotor(s32 channel,u32 command){(void)channel;(void)command;}

extern "C" s32 WPADGetInfoAsync(s32 channel,WPADInfo* output,WPADCallback callback){
    if(!valid(channel)||!output)return WPAD_ERR_INVALID;
    s32 result;bool connected;
    {std::lock_guard<std::mutex> lock(stateMutex);connected=channels[channel].connected;result=connected?WPAD_ERR_NONE:WPAD_ERR_NO_CONTROLLER;if(connected)pendingInfo.push_back({channel,output,callback});}
    if(!connected&&callback)callback(channel,result);
    return result;
}

void nativeSetHostWpadConnected(bool connected){
    WPADConnectCallback callback{};bool changed=false;
    {std::lock_guard<std::mutex> lock(stateMutex);auto& state=channels[WPAD_CHAN0];changed=state.connected!=connected;state.connected=connected;if(changed){state.connectPending=false;state.extensionPending=connected&&state.extension;}callback=state.connect;}
    if(changed&&callback)callback(WPAD_CHAN0,connected?WPAD_ERR_NONE:WPAD_ERR_NO_CONTROLLER);
}

void nativeDispatchWpadCallbacks(){
    struct Completion {InfoRequest request;bool connected;};
    struct ExtensionCompletion {s32 channel;WPADExtensionCallback callback;};
    struct ConnectCompletion {s32 channel;WPADConnectCallback callback;};
    std::vector<Completion> completions;
    std::vector<ExtensionCompletion> extensions;
    std::vector<ConnectCompletion> connections;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        completions.reserve(pendingInfo.size());
        for(const auto& request:pendingInfo)completions.push_back({request,channels[request.channel].connected});
        pendingInfo.clear();
        for(s32 channel=0;channel<WPAD_MAX_CONTROLLERS;++channel){
            auto& state=channels[channel];
            if(state.connectPending&&state.connected&&state.connect)connections.push_back({channel,state.connect});
            state.connectPending=false;
            if(state.extensionPending&&state.connected&&state.extension)extensions.push_back({channel,state.extension});
            state.extensionPending=false;
        }
    }
    // Original WPad::connected installs its extension callback after discovery.
    // Deliver the existing virtual Nunchuk on the frame pump, outside our mutex.
    for(const auto& connection:connections)connection.callback(connection.channel,WPAD_ERR_NONE);
    for(const auto& extension:extensions)extension.callback(extension.channel,WPAD_DEV_FREESTYLE);
    for(const auto& completion:completions){
        const auto& request=completion.request;
        if(completion.connected){WPADInfo info{};info.dpd=1;info.attach=1;info.lowBat=0;info.nearempty=0;info.battery=WPAD_BATTERY_LEVEL_MAX;*request.output=info;}
        if(request.callback)request.callback(request.channel,completion.connected?WPAD_ERR_NONE:WPAD_ERR_NO_CONTROLLER);
    }
}
