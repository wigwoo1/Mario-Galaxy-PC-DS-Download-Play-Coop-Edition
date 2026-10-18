#pragma once
#include "assets.hpp"
#include "Game/System/GameDataPlayerStatus.hpp"
#include "save_chunks.hpp"
// Checked owner of original GameDataPlayerStatus; serialization is Wii-endian.
class NativePlayerStock {
    GameDataPlayerStatus status;
public:
    std::vector<uint8_t> saveBytes()const{return encodeSaveChunks({&status});}
    bool loadBytes(const std::vector<uint8_t>& bytes){if(bytes.size()<16||bytes[1]!=1||be32(bytes.data()+4)!=status.getSignature())return false;GameDataPlayerStatus candidate;try{if(!decodeSaveChunks(bytes,{&candidate}))return false;}catch(const std::exception&){return false;}status=candidate;return true;}
    int getPlayerLeft()const{return status.getPlayerLeft();}
    void addPlayerLeft(int amount){assetCheck(amount>=-99&&amount<=99,"Player stock change outside supported range");status.addPlayerLeft(amount);}
};
inline void playerStockTests(){GameDataPlayerStatus original;original.addPlayerLeft(95);original.addStockedStarPiece(0x1234);original.mStoryProgress=2;std::array<u8,7> bytes{};assetCheck(original.serialize(bytes.data(),bytes.size())==7&&bytes==std::array<u8,7>{2,0,0,0x12,0x34,0,99},"Wii player status byte order mismatch");GameDataPlayerStatus restored;restored.deserialize(bytes.data(),bytes.size());assetCheck(restored.getPlayerLeft()==4&&restored.mStockedStarPiece==0x1234&&restored.mStoryProgress==2&&restored.isPlayerLeftSupply(),"Original player status load policy mismatch");bool rejected=false;try{restored.deserialize(bytes.data(),6);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Truncated player status accepted");NativePlayerStock stock;assetCheck(stock.getPlayerLeft()==4,"Initial player stock mismatch");stock.addPlayerLeft(1);assetCheck(stock.getPlayerLeft()==5,"Player stock increment failed");stock.addPlayerLeft(99);assetCheck(stock.getPlayerLeft()==99,"Player stock cap failed");stock.addPlayerLeft(-99);stock.addPlayerLeft(-1);assetCheck(stock.getPlayerLeft()==0,"Player stock floor failed");}


