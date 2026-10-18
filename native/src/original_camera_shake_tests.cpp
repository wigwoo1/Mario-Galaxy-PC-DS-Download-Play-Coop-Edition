#include <Game/Camera/CameraShakeTask.hpp>
#include <Game/Camera/CameraShakePatternImpl.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
static void near(float a,float b){assert(std::fabs(a-b)<.003f);}
int main(){
 CameraShakePatternSingly pattern(2);pattern.setDirection(TVec2f(3,4));
 near(pattern.mDirection.x,.6f);near(pattern.mDirection.y,.8f);
 CameraShakeTask task(&pattern);assert(task.isEnd());
 TVec2f offset;task.getOffset(&offset);near(offset.x,0);near(offset.y,0);
 task.start(1,0);
 for(unsigned tick=1;tick<=25;++tick){
   task.movement();task.getOffset(&offset);
   double remaining=25-double(tick),pi=3.14159265358979323846;
   double amplitude=2*std::sin(pi*4*remaining/25)*std::sin(pi*.5*remaining/25);
   near(offset.x,float(amplitude*.6));near(offset.y,float(amplitude*.8));
   assert(task.isEnd()==(tick==25));
 }
 task.start(2,3);for(unsigned i=0;i<25;++i)task.movement();
 assert(!task.isEnd() && pattern.mFrame==25);
 task.movement();assert(pattern.mFrame==25);
 task.movement();assert(pattern.mFrame==0);
 for(unsigned i=0;i<25;++i)task.movement();assert(task.isEnd());
 CameraShakePatternVerticalSin wave(3,15);CameraShakeTask infinite(&wave);
 infinite.startInfinity(0);
 for(unsigned i=1;i<=120;++i){
   infinite.movement();assert(!infinite.isEnd());
   infinite.getOffset(&offset);near(offset.x,0);
   near(offset.y,float(-3*std::sin(2*3.14159265358979323846*double(i%15)/15)));
 }
 infinite.endForce();infinite.getOffset(&offset);near(offset.x,0);near(offset.y,0);
 auto frame=wave.mFrame;infinite.movement();assert(wave.mFrame==frame);
 infinite.startInfinity(0);assert(wave.mFrame==0 && !infinite.isEnd());
 std::puts("PASS original camera shake: 25-frame offsets, finite repeats/delay, 120 infinite updates, cancellation and restart");
}
