#pragma once
#include "animation_clock.hpp"
#include "mesh_render.hpp"
#include <chrono>
struct AnimatedPreviewState {
    TextureRenderer* display=nullptr;
    std::function<TextureImage(double)> update;std::function<void(WPARAM)> key;
    std::chrono::steady_clock::time_point last;
    std::string error;size_t painted=0;bool paused=false,minimized=false;void* userContext=nullptr;
};
inline LRESULT CALLBACK animationPreviewProcedure(HWND window,UINT message,WPARAM wp,LPARAM lp){
    auto state=reinterpret_cast<AnimatedPreviewState*>(GetWindowLongPtrW(window,GWLP_USERDATA));
    if(message==WM_NCCREATE){auto cs=reinterpret_cast<CREATESTRUCTW*>(lp);SetWindowLongPtrW(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(cs->lpCreateParams));}
    if(message==WM_KEYDOWN&&wp==VK_ESCAPE){DestroyWindow(window);return 0;}
    if(message==WM_KEYDOWN&&wp==VK_SPACE&&state){if((lp&(1LL<<30))==0){state->paused=!state->paused;state->last=std::chrono::steady_clock::now();}return 0;}
    if(message==WM_KEYDOWN&&wp==VK_OEM_PERIOD&&state&&state->paused&&!state->minimized){if((lp&(1LL<<30))==0){try{state->last=std::chrono::steady_clock::now();state->display->updateImage(state->update(1.0/60));InvalidateRect(window,nullptr,FALSE);}catch(const std::exception& e){state->error=e.what();DestroyWindow(window);}}return 0;}
    if(message==WM_KEYDOWN&&state&&state->key){state->key(wp);return 0;}
    if(message==WM_TIMER&&state){
        auto now=std::chrono::steady_clock::now();double elapsed=std::chrono::duration<double>(now-state->last).count();state->last=now;
        try{if(!state->paused&&!state->minimized){state->display->updateImage(state->update(std::min(elapsed,.25)));InvalidateRect(window,nullptr,FALSE);}}
        catch(const std::exception& e){state->error=e.what();DestroyWindow(window);}return 0;
    }
    if(message==WM_SIZE){if(state)state->minimized=wp==SIZE_MINIMIZED;InvalidateRect(window,nullptr,FALSE);return 0;}
    if(message==WM_PAINT){PAINTSTRUCT paint;BeginPaint(window,&paint);
        if(state&&state->display){RECT r{};GetClientRect(window,&r);try{state->display->present(r.right,r.bottom);++state->painted;}catch(const std::exception& e){state->error=e.what();}}
        EndPaint(window,&paint);if(state&&!state->error.empty())DestroyWindow(window);return 0;}
    if(message==WM_DESTROY){KillTimer(window,1);PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
inline void previewAnimation(BinaryView file,const ModelInfo& model,const ModelRig& rig,const ModelMesh& mesh,const JointAnimation& animation,bool smoke){
    RetainedMeshAppearance retained(file,model,mesh);AnimationClock clock(animation);AnimationTicker ticker;MeshRenderCache cache;bool software=false;
    // Bound startup work for long clips. The fixed camera uses sampled whole-clip bounds.
    MeshCameraBounds camera;size_t steps=std::min<size_t>(240,std::max<size_t>(1,size_t(std::ceil(animation.duration))));
    for(size_t i=0;i<=steps;++i){auto pose=animation.pose(rig,animation.duration*float(i)/float(steps));camera.include(defaultPoseTriangles(model,pose,mesh));}
    auto render=[&](){auto pose=animation.pose(rig,clock.frame);auto triangles=materialPoseTriangles(file,model,pose,mesh,&retained.materials,&retained.vertices);return renderMesh(triangles,software,480,480,&model.textures,&cache,&camera);};
    TextureImage first;try{first=render();}catch(const std::exception&){cache={};software=true;first=render();}
    TextureRenderer display(first,software);auto instance=GetModuleHandleW(nullptr);WNDCLASSW wc{};wc.lpfnWndProc=animationPreviewProcedure;wc.hInstance=instance;
    wc.lpszClassName=L"GalaxyNativeAnimationPreview";wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));assetCheck(RegisterClassW(&wc)!=0,"Animation window registration failed");
    size_t updates=0;AnimatedPreviewState state;state.display=&display;state.last=std::chrono::steady_clock::now();
    state.update=[&](double seconds){ticker.advance(clock,seconds);++updates;return render();};
    HWND window=CreateWindowExW(0,wc.lpszClassName,L"Galaxy native animation preview - Space pauses, Escape closes - not the game",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,720,720,nullptr,nullptr,instance,&state);
    if(!window){UnregisterClassW(wc.lpszClassName,instance);throw std::runtime_error("Animation window creation failed");}
    try{display.attach(window);
        if(smoke){auto previousFrame=clock.frame;SendMessageW(window,WM_SIZE,SIZE_MINIMIZED,0);SendMessageW(window,WM_TIMER,1,0);assetCheck(state.minimized&&clock.frame==previousFrame,"Minimized animation preview advanced playback");SendMessageW(window,WM_SIZE,SIZE_RESTORED,0);assetCheck(!state.minimized,"Animation preview failed to resume after restore");
            auto originalDevice=cache.device.Get();size_t changed=0;
            for(int i=0;i<8;++i){auto next=state.update(1.0/60);for(size_t p=0;p<first.pixels.size();++p)if(!same(first.pixels[p],next.pixels[p]))++changed;
                display.updateImage(next);auto presented=display.readback();
                for(size_t p=0;p<next.pixels.size();++p)assetCheck(same(presented.pixels[p],next.pixels[p]),"Animated display upload mismatch");
                SendMessageW(window,WM_PAINT,0,0);}
            SendMessageW(window,WM_KEYDOWN,VK_SPACE,0);assetCheck(state.paused,"Pause toggle failed");SendMessageW(window,WM_KEYDOWN,VK_SPACE,LPARAM(1LL<<30));assetCheck(state.paused,"Repeated pause key resumed animation");
            auto before=clock.frame;SendMessageW(window,WM_TIMER,1,0);assetCheck(clock.frame==before,"Paused clock advanced");auto previousUpdates=updates;SendMessageW(window,WM_KEYDOWN,VK_OEM_PERIOD,0);assetCheck(state.paused&&updates==previousUpdates+1,"Standalone paused frame step failed");SendMessageW(window,WM_KEYDOWN,VK_OEM_PERIOD,LPARAM(1LL<<30));assetCheck(updates==previousUpdates+1,"Repeated standalone step advanced playback");
            SetWindowPos(window,nullptr,0,0,640,480,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);SendMessageW(window,WM_PAINT,0,0);
            assetCheck(changed>1000&&updates==9&&state.painted==9&&cache.device.Get()==originalDevice&&state.error.empty(),"Animated preview lifecycle failed");DestroyWindow(window);
        }else{ShowWindow(window,SW_SHOW);UpdateWindow(window);state.last=std::chrono::steady_clock::now();assetCheck(SetTimer(window,1,16,nullptr)!=0,"Animation timer failed");
            MSG message{};for(;;){BOOL result=GetMessageW(&message,nullptr,0,0);assetCheck(result!=-1,"Animation message loop failed");if(!result)break;TranslateMessage(&message);DispatchMessageW(&message);}}
        assetCheck(state.error.empty(),state.error.c_str());
    }catch(...){if(IsWindow(window))DestroyWindow(window);UnregisterClassW(wc.lpszClassName,instance);throw;}
    UnregisterClassW(wc.lpszClassName,instance);
}





