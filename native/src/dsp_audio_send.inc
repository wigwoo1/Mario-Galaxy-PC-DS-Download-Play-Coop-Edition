// Native adaptation of the original DSPSendCommands2 control path. The real
// backend owns mail completion; legacy silence remains explicitly distinguished.
int DspCanStartWork();
int DSPSendCommands2(u32* messages,u32 count,void(*callback)(u16)){
    if(!messages)return -1;
    while(!Dsp_Running_Check()){}
    const BOOL interrupts=OSDisableInterrupts();
    if(callback&&!DspCanStartWork()){OSRestoreInterrupts(interrupts);return -1;}
    u16 token=0;
    const s32 result=nativeDspProcessHostAudioCommand(messages,count,&token);
    if(result<0){OSReport("Native DSP HLE rejected command batch (%d)\n",result);OSRestoreInterrupts(interrupts);return -1;}
    int task=0;
    if(result>0){
        if(callback){
            task=DspStartWork(messages[0],callback);
            if(!task){OSRestoreInterrupts(interrupts);return -1;}
            if(result==1)DspFinishWork(token); // Explicit legacy AcceptedSilent only.
        }
        OSRestoreInterrupts(interrupts);
        if(interrupts)nativeDspServiceHostAudio();
        return task;
    }
    if(DSPCheckMailToDSP()){OSRestoreInterrupts(interrupts);return -1;}
    if(callback){task=DspStartWork(messages[0],callback);if(!task){OSRestoreInterrupts(interrupts);return -1;}}
    DSPSendMailToDSP(count);DSPAssertInt();while(DSPCheckMailToDSP()){}
    for(u32 i=0;i<(count?count:1);++i){DSPSendMailToDSP(messages[i]);while(DSPCheckMailToDSP()){}}
    OSRestoreInterrupts(interrupts);return task;
}
