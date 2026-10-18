$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$root=Split-Path $n -Parent
$script=[IO.File]::ReadAllText((Join-Path $n 'test-actor-runtime-dsp-boot-link.ps1'))
$match=[regex]::Match($script,"\`$inputPulse=@'\r?\n([\s\S]*?)\r?\n'@")
if(!$match.Success){throw 'Diagnostic input block not found'}
$pulse=$match.Groups[1].Value
$harness=@"
#include <Native/DiagnosticMenuInput.hpp>
#include <Native/WpadHost.hpp>
#include <cstdio>
#include <cstdlib>
#include <limits>
extern "C" void galaxyNativeBootTrace(const char*){}
void runDiagnosticFrame(){
$pulse
}
int main(){
 KPADInit();WPADInit();const bool ab=std::getenv("GALAXY_DIAGNOSTIC_PRESS_AB"),select=std::getenv("GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT"),create=std::getenv("GALAXY_DIAGNOSTIC_CREATE_PROFILE"),saved=std::getenv("GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT"),start=std::getenv("GALAXY_DIAGNOSTIC_SELECT_START_BUTTON");
 unsigned previous=0,menuClicks=0,confirmClicks=0,startClicks=0;
 for(unsigned stage=0;stage<6;++stage){
  if(stage==1)NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Title);
  if(stage==2)NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Quiet);
  for(unsigned frame=1;frame<=400;++frame){
   // An invalid pointer must not click, even after the stabilization delay.
   if(stage==3)NativeDiagnosticInput::observeMenu(frame>=151,0.25f,-0.5f);
   if(stage==4)NativeDiagnosticInput::observeConfirmation(frame>=101,-0.2f,0.4f,frame>=200);
   if(stage==5)NativeDiagnosticInput::observeStartButton(frame>=101,0.3f,-0.1f,frame>=150);
   runDiagnosticFrame();KPADStatus samples[120]{};const auto count=KPADRead(0,samples,120);
   if(count!=1){std::fprintf(stderr,"FAIL expected one sample, stage %u frame %u count %d\n",stage,frame,count);return 1;}
   const auto& sample=samples[0];
   if(sample.dev_type!=WPAD_DEV_FREESTYLE||sample.data_format!=WPAD_FMT_FREESTYLE_ACC_DPD||sample.acc.y!=1||sample.ex_status.fs.acc.y!=1)return 2;
   const bool press=(stage<2&&frame>=5&&(frame-5)%30==0)||(stage==3&&(select||saved)&&frame==151)||(stage==4&&create&&frame==200)||(stage==5&&start&&frame==150);
   const unsigned held=(press?WPAD_BUTTON_A:0)|((stage==1&&press&&ab)?WPAD_BUTTON_B:0);
   if(sample.hold!=held||sample.trig!=(held&~previous)||sample.release!=(previous&~held)){std::fprintf(stderr,"FAIL button edges, stage %u frame %u: held %x expected %x\n",stage,frame,sample.hold,held);return 3;}
   previous=held;
   const bool pointer=(stage==3&&(select||saved)&&frame==151)||(stage==4&&create&&frame>=101&&frame<=200)||(stage==5&&start&&frame>=101&&frame<=150);
   const float expectedX=stage==4?-0.2f:stage==5?0.3f:0.25f,expectedY=stage==4?0.4f:stage==5?-0.1f:-0.5f;
   if((sample.dpd_valid_fg==2)!=pointer||(pointer&&(sample.pos.x!=expectedX||sample.pos.y!=expectedY))){std::fprintf(stderr,"FAIL pointer state at stage %u frame %u\n",stage,frame);return 4;}
   if(stage==3&&sample.trig&WPAD_BUTTON_A)++menuClicks;
   if(stage==4&&sample.trig&WPAD_BUTTON_A)++confirmClicks;
   if(stage==5&&sample.trig&WPAD_BUTTON_A)++startClicks;
   u32 device=WPAD_DEV_UNKNOWN;if(WPADProbe(0,&device)!=WPAD_ERR_NONE||device!=sample.dev_type||KPADRead(1,samples,120)!=0)return 5;
  }
 }
 if(menuClicks!=((select||saved)?1u:0u)||confirmClicks!=(create?1u:0u)||startClicks!=(start?1u:0u))return 6;
 NativeDiagnosticInput::state()={};NativeDiagnosticInput::observeMenu(true,std::numeric_limits<float>::quiet_NaN(),0);
 if(NativeDiagnosticInput::state().pointerValid)return 7;
 NativeDiagnosticInput::observeMenu(true,1.01f,0);if(NativeDiagnosticInput::state().pointerValid)return 8;
 std::printf("PASS diagnostic input: 2400 frames, single producer, Nunchuk metadata, exact press/release edges, B restricted to title, quiet transitions, invalid pointer delay, %u menu, %u readiness-gated Yes, and %u readiness-gated Start click(s), channel isolation (AB=%u empty=%u create=%u saved=%u start=%u)\n",menuClicks,confirmClicks,startClicks,unsigned(ab),unsigned(select),unsigned(create),unsigned(saved),unsigned(start));
}
"@
$build=Join-Path $n 'build\diagnostic-input-regression';New-Item -ItemType Directory -Force -Path $build|Out-Null
$source=Join-Path $build 'diagnostic-input.cpp';$exe=Join-Path $build 'diagnostic-input.exe'
[IO.File]::WriteAllText($source,$harness,[Text.UTF8Encoding]::new($false))
& (Join-Path $root 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" "-I$root\Petari\libs\RVL_SDK\include" "$n\src\native_kpad_input.cpp" "$n\src\native_wpad_port.cpp" "$n\src\vi_port.cpp" -luser32 $source -o $exe
if($LASTEXITCODE){exit $LASTEXITCODE}
$previousAB=$env:GALAXY_DIAGNOSTIC_PRESS_AB;$previousSelect=$env:GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT;$previousCreate=$env:GALAXY_DIAGNOSTIC_CREATE_PROFILE;$previousSaved=$env:GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT;$previousStart=$env:GALAXY_DIAGNOSTIC_SELECT_START_BUTTON
try{
 foreach($profile in @(@($false,$false,$false,$false,$false),@($true,$false,$false,$false,$false),@($true,$true,$false,$false,$false),@($true,$true,$true,$false,$false),@($true,$false,$false,$true,$false),@($true,$false,$false,$false,$true))){
  if($profile[0]){$env:GALAXY_DIAGNOSTIC_PRESS_AB='1'}else{Remove-Item Env:GALAXY_DIAGNOSTIC_PRESS_AB -ErrorAction SilentlyContinue}
  if($profile[1]){$env:GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT='1'}else{Remove-Item Env:GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT -ErrorAction SilentlyContinue}
  if($profile[2]){$env:GALAXY_DIAGNOSTIC_CREATE_PROFILE='1'}else{Remove-Item Env:GALAXY_DIAGNOSTIC_CREATE_PROFILE -ErrorAction SilentlyContinue}
  if($profile[3]){$env:GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT='1'}else{Remove-Item Env:GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT -ErrorAction SilentlyContinue}
  if($profile[4]){$env:GALAXY_DIAGNOSTIC_SELECT_START_BUTTON='1'}else{Remove-Item Env:GALAXY_DIAGNOSTIC_SELECT_START_BUTTON -ErrorAction SilentlyContinue}
  & $exe
  if($LASTEXITCODE){exit $LASTEXITCODE}
 }
}finally{$env:GALAXY_DIAGNOSTIC_PRESS_AB=$previousAB;$env:GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT=$previousSelect;$env:GALAXY_DIAGNOSTIC_CREATE_PROFILE=$previousCreate;$env:GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT=$previousSaved;$env:GALAXY_DIAGNOSTIC_SELECT_START_BUTTON=$previousStart}
