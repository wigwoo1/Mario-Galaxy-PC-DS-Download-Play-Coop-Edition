#include <JSystem/JAudio2/JASReport.hpp>
#include <string>
int main(){JASReport("%s %d %x", "native-audio",-17,0xabcd);JASReport("%s",std::string(2000,'x').c_str());JASReport(nullptr);}
