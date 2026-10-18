#include "stage_data.hpp"
#include <Game/Demo/DemoTimeKeeper.hpp>
#include <fstream>
#include <memory>
#include <cassert>
#include <cstring>
#include <cstdio>
class JMapInfo;
static std::unique_ptr<StageTable> table;static std::vector<std::string> names;
namespace DemoFunction{s32 createSheetParser(const DemoExecutor*,const char* sheet,JMapInfo**out){assert(!std::strcmp(sheet,"Time"));*out=nullptr;return s32(table->rows);}}
namespace MR{void getCsvDataStrOrNULL(const char**out,const JMapInfo*,const char*key,s32 row){assert(!std::strcmp(key,"PartName"));*out=names.at(row).c_str();}void getCsvDataS32(s32*out,const JMapInfo*,const char*key,s32 row){auto value=table->read(row,table->find(key));*out=s32(std::get<uint32_t>(value));}bool isEqualString(const char*a,const char*b){return a&&b&&!std::strcmp(a,b);}}
static unsigned tablesTested=0;static size_t partsTested=0;static unsigned long long stepsTested=0;void run(const std::filesystem::path& path){names.clear();std::ifstream f(path,std::ios::binary);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)),{});table=std::make_unique<StageTable>(BinaryView(bytes));table->validate();assert(table->rows>0);for(size_t r=0;r<table->rows;++r)names.push_back(std::get<std::string>(table->read(r,table->find("PartName"))));DemoTimeKeeper keeper(nullptr);keeper.start();unsigned updates=0;for(s32 row=0;row<keeper.mNumPartInfos;++row){keeper.setCurrentPart(names[row].c_str());keeper.mCurrentStep=-1;auto&part=keeper.mMainPartInfos[row];assert(part.mTotalStep>0&&part.mTotalStep<100000);for(s32 step=0;step<part.mTotalStep;++step){keeper.update();++updates;assert(keeper._18==row&&keeper.mCurrentStep==step);}assert(keeper.isCurrentDemoPartLastStep());keeper.update();++updates;if(part.mSuspendFlag){assert(keeper._18==row&&keeper.isDemoEnd());}else{assert(keeper._18==row+1);if(row+1<keeper.mNumPartInfos)assert(keeper.mCurrentStep==0&&keeper.mSubPartInfos==keeper.mMainPartInfos+row+1);}}keeper.end();delete[] keeper.mMainPartInfos;++tablesTested;partsTested+=table->rows;stepsTested+=updates;}

int main(int argc,char**argv){assert(argc==2);std::vector<std::filesystem::path> paths;for(const auto& entry:std::filesystem::directory_iterator(std::filesystem::u8path(argv[1])))if(entry.path().extension()==".bcsv")paths.push_back(entry.path());std::sort(paths.begin(),paths.end());assert(paths.size()==138);for(const auto&path:paths){printf("Checking %s\n",path.filename().string().c_str());fflush(stdout);run(path);}printf("PASS original retail timeline execution: %u tables, %zu parts, %llu step updates; test field bridge, not full cutscene playback\n",tablesTested,partsTested,stepsTested);}
