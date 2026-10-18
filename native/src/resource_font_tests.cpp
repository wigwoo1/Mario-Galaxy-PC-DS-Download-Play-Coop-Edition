#include "resource_font.hpp"
#include <cassert>
#include <fstream>
#include <iterator>
#include <cstdio>
int main(int argc,char**argv){
    assert(argc==2);std::ifstream input(argv[1],std::ios::binary);assert(input);
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
    auto font=decodeResourceFont(BinaryView(bytes));assert(!font.sheets.empty());
    for(unsigned code=32;code<127;++code){auto index=font.glyphIndex(uint16_t(code));auto cell=font.glyphCell(index);assert(cell[0]<font.sheets.size());}
    auto rejects=[](const std::vector<uint8_t>& sample){try{decodeResourceFont(BinaryView(sample));return false;}catch(const std::exception&){return true;}};
    auto bad=bytes;bad[6]=0;bad[7]=0;assert(rejects(bad));
    bad=bytes;bad[4]=0xff;bad[5]=0xfe;assert(rejects(bad));
    bad=bytes;bad[12]=0;bad[13]=1;assert(rejects(bad));
    bad=bytes;bad[14]=bad[15]=0;assert(rejects(bad));
    bad=bytes;bad[20]=bad[21]=bad[22]=bad[23]=0;assert(rejects(bad));
    for(size_t length:{size_t(0),size_t(15),bytes.size()-1}){bad.assign(bytes.begin(),bytes.begin()+length);assert(rejects(bad));}
    resourceFontLookupTests();
    std::printf("PASS original font decoding: %zu sheets, %zu width entries; ASCII lookup and malformed-header/block rejection\n",font.sheets.size(),font.widths.size());
}
