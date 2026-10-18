#include "layout_composition.hpp"
#pragma once
#include "diagnostic_body.hpp"
#include "stage_scene.hpp"
#include "stage_collision.hpp"
#include "animation_preview.hpp"
#include "gravity_body.hpp"
#include "native_scene.hpp"
#include "scenario_actors.hpp"
#include "body_model.hpp"
#include "diagnostic_heading.hpp"
#include "native_gamepad.hpp"
struct BodyPreviewState {AnimatedPreviewState display;bool forward=false,back=false,left=false,right=false,jump=false,slow=false,reset=false,retry=false;};
inline LRESULT CALLBACK bodyPreviewProcedure(HWND window,UINT message,WPARAM wp,LPARAM lp){
    auto displayState=reinterpret_cast<AnimatedPreviewState*>(GetWindowLongPtrW(window,GWLP_USERDATA));
    auto state=displayState?static_cast<BodyPreviewState*>(displayState->userContext):nullptr;
    if(state&&(message==WM_KEYDOWN||message==WM_KEYUP)){bool down=message==WM_KEYDOWN;
        if(wp==VK_SHIFT)state->slow=down;if(wp=='W')state->forward=down;if(wp=='S')state->back=down;if(wp=='A')state->left=down;if(wp=='D')state->right=down;
        if(wp=='P'&&down&&(lp&(1LL<<30))==0){state->display.paused=!state->display.paused;state->display.last=std::chrono::steady_clock::now();state->jump=false;return 0;}
        if(wp=='T'&&down&&(lp&(1LL<<30))==0){state->retry=true;return 0;}
        if(wp=='R'&&down&&(lp&(1LL<<30))==0){state->reset=true;return 0;}
        if(wp==VK_SPACE){if(down&&(lp&(1LL<<30))==0)state->jump=true;return 0;}}
    if(state&&message==WM_KILLFOCUS){state->forward=state->back=state->left=state->right=state->jump=state->slow=state->reset=state->retry=false;}
    return animationPreviewProcedure(window,message,wp,lp);
}
inline void previewDiagnosticBody(const StageScene& scene,const StageCollision& collision,bool smoke,const ZoneGravityField* field=nullptr,const CollisionPoint* spawn=nullptr,NativeScenarioActors* scenarioActors=nullptr,DiagnosticMarioModel* mario=nullptr,const CollisionPoint* initialFacing=nullptr,const std::function<std::optional<AffineMatrix>()>& restartPose={},NativeRuntimeClient* originalRuntime=nullptr){
    if(mario)mario->enableDiagnosticCadence();if(smoke&&mario)mario->verify();if(smoke&&scenarioActors)scenarioActors->verifySwitchDispatch();
    CollisionIndex index(collision.geometry);const auto& triangle=collision.geometry.triangles.front();DiagnosticBody body;CollisionPoint gravity{},up{};
    float normalLength=std::sqrt(collisionDot(triangle.normal,triangle.normal));assetCheck(normalLength>0,"Invalid body spawn normal");
    for(size_t k=0;k<3;++k){up[k]=triangle.normal[k]/normalLength;gravity[k]=-100*up[k];body.position[k]=(triangle.points[0][k]+triangle.points[1][k]+triangle.points[2][k])/3+30*up[k];}
    if(field&&spawn){body.position=*spawn;gravity=gravityBodyVector(*field,body.position);for(size_t k=0;k<3;++k){up[k]=-gravity[k]/100;body.position[k]+=up[k]*body.radius;}}
    CollisionPoint axis=std::abs(up[1])<.9f?CollisionPoint{0,1,0}:CollisionPoint{1,0,0};auto right=collisionCross(axis,up);float rightLength=std::sqrt(collisionDot(right,right));for(float& v:right)v/=rightLength;auto forward=collisionCross(up,right);DiagnosticHeading movementHeading;movementHeading.update(up,forward,forward);
    MeshCameraBounds camera;std::vector<RenderVertex> corners;for(int x:{-1,1})for(int y:{-1,1})for(int z:{-1,1})corners.push_back({{body.position[0]+x*200,body.position[1]+y*200,body.position[2]+z*200},{0,1,0}});camera.include(corners);
    const auto originalCamera=camera;const auto cameraOrigin=body.position;const auto initialBody=body;
    MeshRenderCache cache;bool software=false;DiagnosticHeading heading;heading.update(up,initialFacing?*initialFacing:CollisionPoint{0,0,0},forward);const auto initialHeading=heading;const auto initialMovementHeading=movementHeading;
    auto textures=scene.textures;int marioTextureOffset=int(textures.size());if(mario)textures.insert(textures.end(),mario->textures().begin(),mario->textures().end());
    bool fatalPending=false,fatalCompletionObserved=false,fatalCompletionPresented=false;
    auto render=[&](){
        if(scenarioActors)scenarioActors->renderPhases();
        auto delta=body.position;for(size_t k=0;k<3;++k)delta[k]-=cameraOrigin[k];float x=delta[0],z=delta[2];delta[0]=std::cos(.35f)*x-std::sin(.35f)*z;delta[2]=std::sin(.35f)*x+std::cos(.35f)*z;
        for(size_t k=0;k<3;++k){camera.low[k]=originalCamera.low[k]+delta[k];camera.high[k]=originalCamera.high[k]+delta[k];}
        auto vertices=scenarioActors?stageSceneWithCoinRotation(scene,scenarioActors->coinRotater().normal,[&](const std::string& key){return scenarioActors->coinVisible(key);}):scene.triangles;const CollisionPoint offsets[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        const size_t faces[][3]={{4,0,2},{4,2,1},{4,1,3},{4,3,0},{5,2,0},{5,1,2},{5,3,1},{5,0,3}};
        if(mario){auto visual=mario->triangles(body.position,up,heading.right,heading.forward,body.radius,marioTextureOffset);vertices.insert(vertices.end(),visual.begin(),visual.end());}
        else for(auto& face:faces)for(size_t j:face){RenderVertex vertex;for(size_t k=0;k<3;++k)vertex.position[k]=body.position[k]+offsets[j][k]*body.radius;vertex.normal=offsets[j];vertex.color={1,.55f,.1f,1};vertices.push_back(vertex);}
        auto frame=renderMesh(vertices,software,480,480,&textures,&cache,&camera);if(scenarioActors)scenarioActors->composeDeathUi(frame);if(fatalPending&&fatalCompletionObserved)fatalCompletionPresented=true;return frame;};
    TextureImage first;try{first=render();}catch(const std::exception&){cache={};software=true;first=render();}TextureRenderer display(first,software);
    BodyPreviewState state;state.display.display=&display;state.display.userContext=&state;state.display.last=std::chrono::steady_clock::now();double remainder=0;size_t ticks=0;
    uint64_t initialCoinTicks=scenarioActors?scenarioActors->coinRotater().ticks:0;
    std::vector<size_t> initialActorUpdates;if(scenarioActors)for(auto actor:scenarioActors->actors)initialActorUpdates.push_back(actor->updates);
    auto scheduleFatalRetry=[&](){if(!fatalPending&&scenarioActors&&restartPose&&scenarioActors->beginFatalSequenceAt(body.position)){fatalPending=true;fatalCompletionObserved=fatalCompletionPresented=false;state.retry=state.reset=state.jump=false;body.velocity={};return true;}return false;};
    NativeGamepad gamepad;HWND activeWindow=nullptr;NativeScene runtime;runtime.create<NativeCallbackActor>([&](){
        if(scenarioActors)scenarioActors->tick();bool completingDeathRetry=false;
        if(fatalPending){state.retry=state.reset=state.jump=false;auto& death=scenarioActors->playerDeathSequence();if(death.running()||death.services.endGameOver){++ticks;return;}assetCheck(death.services.endMiss==1,"Fatal sequence ended without a completion event");if(!fatalCompletionObserved){fatalCompletionObserved=true;++ticks;return;}if(!fatalCompletionPresented){++ticks;return;}state.retry=true;fatalPending=false;fatalCompletionObserved=fatalCompletionPresented=false;completingDeathRetry=true;}
        if(state.reset){body=initialBody;heading=initialHeading;movementHeading=initialMovementHeading;right=collisionCross(movementHeading.forward,up);forward=movementHeading.forward;state.jump=false;state.reset=false;}
        if(state.retry){
            assetCheck(bool(restartPose)&&field,"Checkpoint retry requires scenario start/gravity metadata");auto pose=restartPose();assetCheck(pose.has_value(),"Checkpoint retry has no authored start");
            DiagnosticBody restarted;restarted.radius=body.radius;restarted.position={(*pose)[3],(*pose)[7],(*pose)[11]};auto restartGravity=gravityBodyVector(*field,restarted.position);CollisionPoint restartUp{};for(size_t k=0;k<3;++k){restartUp[k]=-restartGravity[k]/100;restarted.position[k]+=restartUp[k]*restarted.radius;}
            auto restartAxis=std::abs(restartUp[1])<.9f?CollisionPoint{0,1,0}:CollisionPoint{1,0,0};auto restartRight=collisionCross(restartAxis,restartUp);auto restartForward=collisionCross(restartUp,restartRight);CollisionPoint authoredFacing{(*pose)[2],(*pose)[6],(*pose)[10]};
            DiagnosticHeading restartedHeading,restartedMovement;restartedHeading.update(restartUp,authoredFacing,restartForward);restartedMovement.update(restartUp,{0,0,0},restartForward);
            body=restarted;heading=restartedHeading;movementHeading=restartedMovement;state.jump=false;state.retry=false;if(completingDeathRetry)scenarioActors->finishDiagnosticDeathRetry();
        }
        if(field){gravity=gravityBodyVector(*field,body.position);for(size_t k=0;k<3;++k)up[k]=-gravity[k]/100;axis=std::abs(up[1])<.9f?CollisionPoint{0,1,0}:CollisionPoint{1,0,0};right=collisionCross(axis,up);rightLength=std::sqrt(collisionDot(right,right));for(float& v:right)v/=rightLength;auto fallback=collisionCross(up,right);movementHeading.update(up,{0,0,0},fallback);right=movementHeading.right;for(float& value:right)value=-value;forward=movementHeading.forward;}
        auto pad=gamepad.read(!smoke&&activeWindow&&GetForegroundWindow()==activeWindow);auto keyboard=nativeKeyboardVector(int(state.right)-int(state.left),int(state.forward)-int(state.back),state.slow);float horizontal=keyboard[0],vertical=keyboard[1];if(horizontal==0&&vertical==0){horizontal=pad.stick[0];vertical=pad.stick[1];}if(pad.jump)state.jump=true;CollisionPoint input{};for(size_t k=0;k<3;++k)input[k]=right[k]*horizontal+forward[k]*vertical;body.tick(index,gravity,input,state.jump,originalRuntime);if(scenarioActors){scenarioActors->collectCoinsAt(body.position,body.radius);scenarioActors->updatePlayerCheckpoint(body.position,body.grounded);scheduleFatalRetry();}if(mario){heading.update(up,input,forward);mario->tick(collisionDot(input,input)>0,body.grounded,collisionDot(body.velocity,up),std::min(1.f,std::sqrt(collisionDot(input,input))));}state.jump=false;++ticks;});
    state.display.update=[&](double seconds){remainder+=seconds*60;while(remainder>=1){runtime.tick();remainder-=1;}return render();};
    auto instance=GetModuleHandleW(nullptr);WNDCLASSW wc{};wc.lpfnWndProc=bodyPreviewProcedure;wc.hInstance=instance;wc.lpszClassName=L"GalaxyDiagnosticBody";wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));assetCheck(RegisterClassW(&wc)!=0,"Body window registration failed");
    HWND window=CreateWindowExW(0,wc.lpszClassName,mario?L"Galaxy Mario model on diagnostic body - WASD, Space jumps, R resets, T retries checkpoint, P pauses, period steps - not original Mario gameplay":L"Galaxy diagnostic sphere - WASD moves, Space jumps, R resets, T retries checkpoint, P pauses, period steps, Escape closes - not Mario gameplay",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,720,720,nullptr,nullptr,instance,&state.display);
    activeWindow=window;
    if(!window){UnregisterClassW(wc.lpszClassName,instance);throw std::runtime_error("Body window creation failed");}
    try{display.attach(window);if(smoke){SendMessageW(window,WM_KEYDOWN,'W',0);assetCheck(state.forward,"Body input press failed");SendMessageW(window,WM_KILLFOCUS,0,0);assetCheck(!state.forward,"Body focus loss failed");
            auto device=cache.device.Get();for(float& value:movementHeading.forward)value=-value;for(float& value:movementHeading.right)value=-value;body.velocity={100,100,100};for(size_t k=0;k<3;++k)body.position[k]+=up[k]*50;SendMessageW(window,WM_KEYDOWN,'R',0);assetCheck(state.reset,"Body reset key failed");auto pausedPosition=body.position;SendMessageW(window,WM_KEYDOWN,'P',0);SendMessageW(window,WM_TIMER,1,0);assetCheck(state.display.paused&&ticks==0&&body.position==pausedPosition&&state.reset,"Paused body diagnostic advanced simulation");SendMessageW(window,WM_KEYDOWN,VK_OEM_PERIOD,0);assetCheck(state.display.paused&&ticks==1&&!state.reset,"Paused body frame step failed");SendMessageW(window,WM_KEYDOWN,VK_OEM_PERIOD,LPARAM(1LL<<30));assetCheck(ticks==1,"Repeated step key advanced simulation");SendMessageW(window,WM_KEYDOWN,'P',0);assetCheck(!state.display.paused,"Body diagnostic pause failed to resume");
            if(mario){SendMessageW(window,WM_KEYDOWN,'D',0);SendMessageW(window,WM_KEYDOWN,VK_SHIFT,0);assetCheck(state.slow,"Slow walk key failed");}
            for(int i=0;i<8;++i){auto frame=state.display.update(1.0/60);if(i==0){auto resetDelta=body.position;for(size_t k=0;k<3;++k)resetDelta[k]-=initialBody.position[k];assetCheck(!state.reset&&collisionDot(resetDelta,resetDelta)<1,"Body reset did not restore spawn before movement");assetCheck(collisionDot(movementHeading.forward,initialMovementHeading.forward)>.999f,"Body reset retained displaced movement orientation");}display.updateImage(frame);auto readback=display.readback();for(size_t p=0;p<frame.pixels.size();++p)assetCheck(same(frame.pixels[p],readback.pixels[p]),"Body display pixel mismatch");SendMessageW(window,WM_PAINT,0,0);}
            for(size_t k=0;k<3;++k)assetCheck(std::abs((camera.high[k]-camera.low[k])-(originalCamera.high[k]-originalCamera.low[k]))<.001,"Following camera changed scale");
            if(mario){SendMessageW(window,WM_KEYUP,VK_SHIFT,0);assetCheck(!state.slow,"Slow walk key release failed");SendMessageW(window,WM_KEYUP,'D',0);assetCheck(!state.right&&collisionDot(heading.forward,right)>.99,"Mario diagnostic heading did not follow directional input");}
            SendMessageW(window,WM_KEYDOWN,'W',0);SendMessageW(window,WM_KEYUP,'W',0);assetCheck(!state.forward,"Body input release failed");
            size_t expectedTicks=9;
            if(originalRuntime){
 for(int i=0;i<120;++i){runtime.tick();++expectedTicks;}
 assetCheck(originalRuntime->contactCalls>0,"Rendered original Binder never contacted terrain");
 for(float value:body.position)assetCheck(std::isfinite(value),"Nonfinite original runtime body position");
 double overlap=0;for(const auto& contact:sphereContacts(collision.geometry,body.position,body.radius))overlap=std::max(overlap,contact.penetration);assetCheck(overlap<.05f,"Rendered original Binder body remains inside terrain");
 auto frame=render();display.updateImage(frame);auto readback=display.readback();for(size_t p=0;p<frame.pixels.size();++p)assetCheck(same(frame.pixels[p],readback.pixels[p]),"Original runtime contact frame pixel mismatch");
}
            if(restartPose&&scenarioActors&&field&&scenarioActors->restartCubeCount()>0){
                bool triggered=false;for(size_t i=0;i<scenarioActors->restartCubeCount();++i){const auto& cube=scenarioActors->restartCube(i);if(cube.isValid()){scenarioActors->updatePlayerCheckpoint(cube.interior(),true);triggered=true;break;}}
                assetCheck(triggered,"Checkpoint smoke has no active trigger");auto pose=restartPose();assetCheck(pose.has_value(),"Checkpoint smoke has no target");CollisionPoint target{(*pose)[3],(*pose)[7],(*pose)[11]};auto targetGravity=gravityBodyVector(*field,target);for(size_t k=0;k<3;++k)target[k]-=targetGravity[k]/100*body.radius;
                body.velocity={100,100,100};state.jump=true;SendMessageW(window,WM_KEYDOWN,'P',0);SendMessageW(window,WM_KEYDOWN,'T',0);assetCheck(state.retry,"Checkpoint retry key failed");SendMessageW(window,WM_KEYDOWN,VK_OEM_PERIOD,0);++expectedTicks;
                CollisionPoint difference{};for(size_t k=0;k<3;++k)difference[k]=body.position[k]-target[k];assetCheck(!state.retry&&!state.jump&&collisionDot(difference,difference)<1&&collisionDot(body.velocity,body.velocity)<100,"Checkpoint retry did not restore authored position/clear motion");CollisionPoint targetUp{},targetFacing{(*pose)[2],(*pose)[6],(*pose)[10]};for(size_t k=0;k<3;++k)targetUp[k]=-targetGravity[k]/100;DiagnosticHeading expectedHeading;expectedHeading.update(targetUp,targetFacing,forward);assetCheck(collisionDot(heading.forward,expectedHeading.forward)>.99f,"Checkpoint retry did not restore authored facing");
            }
            if(restartPose&&scenarioActors&&field&&scenarioActors->fatalAreaCount()>0){
                bool triggered=false;for(size_t i=0;i<scenarioActors->fatalAreaCount();++i){auto point=scenarioActors->fatalArea(i).interior();if(scenarioActors->evaluateFatalArea(i,point)){body.position=point;triggered=scheduleFatalRetry();break;}}
                assetCheck(triggered&&fatalPending&&!state.retry,"Authored fatal event bypassed death sequence");auto fatalPosition=body.position;int lives=scenarioActors->playerLives();auto pose=restartPose();CollisionPoint target{(*pose)[3],(*pose)[7],(*pose)[11]};auto targetGravity=gravityBodyVector(*field,target);for(size_t k=0;k<3;++k)target[k]-=targetGravity[k]/100*body.radius;
                state.forward=state.jump=true;SendMessageW(window,WM_KEYDOWN,'T',0);SendMessageW(window,WM_KEYDOWN,'R',0);SendMessageW(window,WM_KEYDOWN,VK_OEM_PERIOD,0);++expectedTicks;assetCheck(body.position==fatalPosition&&fatalPending&&!state.retry&&!state.reset&&scenarioActors->playerLives()==lives,"Fatal demo allowed motion/reset/retry or early life loss");state.forward=false;
                unsigned deathTicks=1;while(!fatalCompletionObserved&&deathTicks<512){runtime.tick();++expectedTicks;++deathTicks;assetCheck(body.position==fatalPosition,"Fatal demo moved diagnostic body");}
                assetCheck(fatalCompletionObserved&&fatalPending&&deathTicks>120&&!scenarioActors->playerDeathSequence().running()&&scenarioActors->playerLives()==lives-1&&scenarioActors->playerDeathSequence().services.lifeDecrements==1,"Staged death timing/life/completion mismatch");for(int catchup=0;catchup<3;++catchup){runtime.tick();++expectedTicks;assetCheck(fatalPending&&!fatalCompletionPresented&&body.position==fatalPosition,"Catch-up updates skipped closed wipe presentation");}auto closed=render();for(auto pixel:closed.pixels)assetCheck(pixel.r==0&&pixel.g==0&&pixel.b==0,"Staged death did not present black closing frame");display.updateImage(closed);auto closedReadback=display.readback();for(size_t p=0;p<closed.pixels.size();++p)assetCheck(same(closed.pixels[p],closedReadback.pixels[p]),"Closing wipe display upload mismatch");
                runtime.tick();++expectedTicks;CollisionPoint difference{};for(size_t k=0;k<3;++k)difference[k]=body.position[k]-target[k];assetCheck(!fatalPending&&!state.retry&&collisionDot(difference,difference)<1&&collisionDot(body.velocity,body.velocity)<100,"Staged fatal retry failed to restore checkpoint");TextureImage opened{608,456,6,std::vector<Pixel>(608*456,{200,160,100,255})};assetCheck(scenarioActors->composeDeathUi(opened)==0,"Checkpoint retry retained death UI");
            }
            if(originalRuntime)assetCheck(originalRuntime->bindCalls==ticks,"Rendered body did not use original Binder for every movement update");
            assetCheck(ticks==expectedTicks&&state.display.painted==8&&state.display.error.empty()&&body.partialSteps==0&&cache.device.Get()==device,"Body preview smoke failed");
            if(scenarioActors)assetCheck(scenarioActors->coinRotater().ticks==initialCoinTicks+ticks,"Scenario coin rotation/body cadence mismatch");if(scenarioActors)for(size_t i=0;i<scenarioActors->actors.size();++i){auto actor=scenarioActors->actors[i];assetCheck(actor->updates==initialActorUpdates[i]+ticks,"Scenario actor/body update cadence mismatch");assetCheck(actor->animations==actor->views&&actor->views==actor->draws&&actor->draws>=9,"Scenario render phases cadence mismatch");}DestroyWindow(window);
        }else{ShowWindow(window,SW_SHOW);UpdateWindow(window);assetCheck(SetTimer(window,1,16,nullptr)!=0,"Body timer failed");MSG message{};for(;;){BOOL result=GetMessageW(&message,nullptr,0,0);assetCheck(result!=-1,"Body message loop failed");if(!result)break;TranslateMessage(&message);DispatchMessageW(&message);}}
        assetCheck(state.display.error.empty(),state.display.error.c_str());
    }catch(...){if(IsWindow(window))DestroyWindow(window);UnregisterClassW(wc.lpszClassName,instance);throw;}
    UnregisterClassW(wc.lpszClassName,instance);
}



























