#include <JSystem/JUtility/JUTConsole.hpp>
#include <JSystem/JUtility/JUTException.hpp>
int main(){JUTReportConsole_f("report=%d\n",12);JUTWarningConsole_f("warning=%d\n",34);JUTException::panic_f("jut-fixture.cpp",56,"panic=%d",78);}
