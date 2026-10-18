#pragma once
#include "native_disc_archive_loader.hpp"
#include "native_message_records.hpp"
#include <Native/GameMessages.hpp>
#include <Game/System/ErrorArchive.hpp>
// Holder must outlive backend, or be released while its consumers are quiescent.
class NativeDiscMessageBackend final:public NativeGameMessageBackend {
    NativeDiscArchiveLoader& disc;
    struct State{std::unique_ptr<NativeMessageRecords> system,game;};
    std::map<MessageHolder*,State> owners;
    static std::unique_ptr<NativeMessageRecords> decode(OwnedArchiveResources& archive){
        auto bmg=archive.get("/message.bmg"),ids=archive.get("/messageid.tbl");assetCheck(bmg && ids,"Missing native message resources");
        auto records=std::make_unique<NativeMessageRecords>(std::vector<uint8_t>(bmg,bmg+archive.sizeOf(bmg)));records->attachIds(std::vector<uint8_t>(ids,ids+archive.sizeOf(ids)));return records;
    }
public:
    explicit NativeDiscMessageBackend(NativeDiscArchiveLoader& source):disc(source){}
    ~NativeDiscMessageBackend()override{nativeUnbindGameMessageBackend(this);while(!owners.empty())release(*owners.begin()->first);}
    void load(MessageHolder& holder,bool system)override{
        assetCheck(system?!holder.mSystemMessageData:!holder.mGameMessageData,"Message holder already populated");
        std::unique_ptr<NativeMessageRecords> candidate;
        if(system){auto size=be32(cErrorArchive+4);assetCheck(size<=sizeof(cErrorArchive),"Invalid embedded archive");OwnedArchiveResources error(std::vector<uint8_t>(cErrorArchive,cErrorArchive+size));auto data=error.get("/usenglish/messagedata/system.arc");assetCheck(data,"Missing embedded system messages");OwnedArchiveResources nested(std::vector<uint8_t>(data,data+error.sizeOf(data)));candidate=decode(nested);}
        else{auto archive=disc.mount("/UsEnglish/MessageData/Message.arc");candidate=decode(*archive);}
        auto& state=owners[&holder];if(system){state.system=std::move(candidate);holder.mSystemMessageData=&state.system->view;}else{state.game=std::move(candidate);holder.mGameMessageData=&state.game->view;}
    }
    bool release(MessageHolder& holder){auto found=owners.find(&holder);if(found==owners.end())return false;nativeUnbindSceneMessages(holder);auto& state=found->second;if(state.system && holder.mSystemMessageData==&state.system->view)holder.mSystemMessageData=nullptr;if(state.game){if(holder.mGameMessageData==&state.game->view)holder.mGameMessageData=nullptr;if(holder.mSceneMessageData==&state.game->view)holder.mSceneMessageData=nullptr;}owners.erase(found);return true;}
};
