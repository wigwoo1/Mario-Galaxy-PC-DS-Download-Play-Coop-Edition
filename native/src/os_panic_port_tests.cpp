extern "C" void OSPanic(const char*,int,const char*,...);
int main(){OSPanic("native-panic-fixture.cpp",42,"failure %s=%d","code",17);return 0;}
