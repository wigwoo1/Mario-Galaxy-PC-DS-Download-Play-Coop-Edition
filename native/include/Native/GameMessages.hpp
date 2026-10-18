#pragma once
class MessageHolder;
class NativeGameMessageBackend{public:virtual ~NativeGameMessageBackend()=default;virtual void load(MessageHolder&,bool system)=0;};
void nativeBindGameMessageBackend(NativeGameMessageBackend*);
void nativeUnbindGameMessageBackend(NativeGameMessageBackend*);
void nativeInitGameMessages(MessageHolder&,bool system);
class MessageData;
void nativeBindSceneMessages(MessageHolder&);
void nativeUnbindSceneMessages(MessageHolder&);
MessageData* nativeSceneMessages();
