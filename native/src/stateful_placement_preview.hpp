#pragma once
#include "stateful_placement_render.hpp"
#include "animation_preview.hpp"
#include "bck_settings_table.hpp"
#include "mario_animation_groups.hpp"
inline void previewStatefulPlacement(std::shared_ptr<NativeJmapTable> table,size_t row,BinaryView file,const ModelInfo& model,const ModelRig& rig,const ModelMesh& mesh,bool smoke,const JointAnimation* animation=nullptr,const BckCtrlData* settings=nullptr,NativeMarioGroupPlayer* group=nullptr,std::function<void()> beforeTick={},std::function<void(WPARAM)> onKey={}){
    std::unique_ptr<AnimationClock> clock;if(animation)clock=std::make_unique<AnimationClock>(*animation);
    if(clock&&settings)applyBckSettings(*clock,*settings);float initialAnimation=clock?clock->frame:0;
    struct Rotation:Nerve {std::function<void()> beforeTick;NativeMarioGroupPlayer* group=nullptr;AnimationClock* clock=nullptr;void execute(Spine* spine)const override{auto actor=static_cast<NativeStatefulPlacedActor*>(static_cast<LiveActor*>(spine->mExecutor));if(beforeTick)beforeTick();if(clock)clock->tick();if(group)group->tick();actor->localRotationDegrees[1]=std::fmod(actor->localRotationDegrees[1]+3,360.f);}} rotation;rotation.clock=clock.get();rotation.group=group;rotation.beforeTick=beforeTick;
    NativeScene scene;auto& actor=scene.place<NativeStatefulPlacedActor>(JMapInfoIter(&table->info,int(row)),table,identityMatrix(),rotation);MeshRenderCache cache;bool software=false;double remainder=0;
    RetainedMeshAppearance retained(file,model,mesh);auto local=materialPoseTriangles(file,model,rig,mesh,&retained.materials,&retained.vertices);auto geometry=[&](){auto triangles=group?materialPoseTriangles(file,model,group->pose(),mesh,&retained.materials,&retained.vertices):animation?materialPoseTriangles(file,model,animation->pose(rig,clock->frame),mesh,&retained.materials,&retained.vertices):local;for(auto& vertex:triangles){vertex.position=transformPoint(actor.world,vertex.position);vertex.normal=skinNormal(actor.world,vertex.normal,vertex.normal);}return triangles;};
    MeshCameraBounds camera;float initial=actor.localRotationDegrees[1];for(int i=0;i<12;++i){if(animation)clock->frame=clock->start+(clock->end-clock->start)*float(i)/12;actor.localRotationDegrees[1]=initial+i*30;actor.syncTransform();camera.include(geometry());}actor.localRotationDegrees[1]=initial;if(clock)clock->frame=initialAnimation;actor.syncTransform();
    auto render=[&](){return renderMesh(geometry(),software,480,480,&model.textures,&cache,&camera);};
    TextureImage first;try{first=render();}catch(const std::exception&){cache={};software=true;first=render();}
    TextureRenderer display(first,software);auto instance=GetModuleHandleW(nullptr);WNDCLASSW wc{};wc.lpfnWndProc=animationPreviewProcedure;wc.hInstance=instance;
    wc.lpszClassName=L"GalaxyNativeStatefulPlacementPreview";wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));assetCheck(RegisterClassW(&wc)!=0,"Animation window registration failed");
    size_t updates=0;AnimatedPreviewState state;state.key=onKey;state.display=&display;state.last=std::chrono::steady_clock::now();
    state.update=[&](double seconds){assetCheck(std::isfinite(seconds)&&seconds>=0&&seconds<=.25,"Invalid actor preview elapsed time");remainder+=seconds*60;size_t ticks=size_t(std::floor(remainder+1e-9));remainder-=ticks;if(remainder<0)remainder=0;for(size_t i=0;i<ticks;++i)scene.tick();++updates;return render();};
    HWND window=CreateWindowExW(0,wc.lpszClassName,L"Galaxy actor diagnostic - 1 wait / 2 run / 3 Jump / 4 group, Space pause, Esc close - not the game",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,720,720,nullptr,nullptr,instance,&state);
    if(!window){UnregisterClassW(wc.lpszClassName,instance);throw std::runtime_error("Animation window creation failed");}
    try{display.attach(window);
        if(smoke){
            auto originalDevice=cache.device.Get();size_t changed=0;
            for(int i=0;i<8;++i){if(onKey)SendMessageW(window,WM_KEYDOWN,i<2?'2':i<4?'3':i==4?'4':'1',0);auto next=state.update(1.0/60);for(size_t p=0;p<first.pixels.size();++p)if(!same(first.pixels[p],next.pixels[p]))++changed;
                display.updateImage(next);auto presented=display.readback();
                for(size_t p=0;p<next.pixels.size();++p)assetCheck(same(presented.pixels[p],next.pixels[p]),"Animated display upload mismatch");
                SendMessageW(window,WM_PAINT,0,0);}
            SendMessageW(window,WM_KEYDOWN,VK_SPACE,0);assetCheck(state.paused,"Pause toggle failed");
            auto groupBefore=group?group->trackClocks():std::vector<AnimationClock>{};auto before=actor.getNerveStep();float beforeAnimation=clock?clock->frame:0;SendMessageW(window,WM_TIMER,1,0);assetCheck(actor.getNerveStep()==before&&(!clock||clock->frame==beforeAnimation),"Paused clock advanced");
            if(group){for(size_t i=0;i<groupBefore.size();++i)assetCheck(group->trackClocks()[i].frame==groupBefore[i].frame,"Paused group clock advanced");}SetWindowPos(window,nullptr,0,0,640,480,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);SendMessageW(window,WM_PAINT,0,0);
            if(animation){AnimationClock expected(*animation);if(settings)applyBckSettings(expected,*settings);for(int i=0;i<8;++i)expected.tick();assetCheck(clock->frame==expected.frame&&clock->state==expected.state,"Actor-driven animation clock diverged");auto initialPose=animation->pose(rig,initialAnimation),currentPose=animation->pose(rig,clock->frame);bool poseChanged=false;for(size_t j=0;j<initialPose.joints.size();++j){auto& a=initialPose.joints[j];auto& b=currentPose.joints[j];poseChanged|=a.scale!=b.scale||a.rotation!=b.rotation||a.translation!=b.translation;}assetCheck(poseChanged,"Actor-driven animation pose did not change");}
            assetCheck(changed>1000&&updates==8&&actor.getNerveStep()==8&&state.painted==9&&cache.device.Get()==originalDevice&&state.error.empty(),"Animated preview lifecycle failed");DestroyWindow(window);
        }else{ShowWindow(window,SW_SHOW);UpdateWindow(window);state.last=std::chrono::steady_clock::now();assetCheck(SetTimer(window,1,16,nullptr)!=0,"Animation timer failed");
            MSG message{};for(;;){BOOL result=GetMessageW(&message,nullptr,0,0);assetCheck(result!=-1,"Animation message loop failed");if(!result)break;TranslateMessage(&message);DispatchMessageW(&message);}}
        assetCheck(state.error.empty(),state.error.c_str());
    }catch(...){if(IsWindow(window))DestroyWindow(window);UnregisterClassW(wc.lpszClassName,instance);throw;}
    UnregisterClassW(wc.lpszClassName,instance);
}








