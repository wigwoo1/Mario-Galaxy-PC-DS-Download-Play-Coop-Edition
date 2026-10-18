#include <stdint.h>
void JKRDefaultMemoryErrorRoutine(void*,uint32_t,int);
int main(){JKRDefaultMemoryErrorRoutine(nullptr,4096,32);}
