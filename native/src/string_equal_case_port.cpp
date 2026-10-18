#include <strings.h>
namespace MR { bool isEqualStringCase(const char* a,const char* b){return strcasecmp(a,b)==0;} }
namespace MR {bool isEqualString(const char* a,const char* b){return strcmp(a,b)==0;}}
