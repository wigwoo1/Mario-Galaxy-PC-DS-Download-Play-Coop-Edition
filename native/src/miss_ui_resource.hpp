#pragma once
#include "message_text.hpp"
#include "resource_font.hpp"
#include "layout_text_binding.hpp"
#include "layout_pose.hpp"
struct NativeMissUiResources{NativeLayoutResource layout;std::vector<NativeResourceFont> fonts;std::map<std::string,NativeLayoutAnimation> animations;};
inline NativeMissUiResources loadNativeMissUi(WiiAssets& assets){
    Rarc archive(assets.readFile("/LayoutData/TooBad.arc"));auto bytes=archive.readFile("/blyt/toobad.brlyt");NativeMissUiResources result;result.layout=decodeLayoutResource(BinaryView(bytes));Rarc messages(assets.readFile("/UsEnglish/MessageData/Message.arc"));NativeMessageCatalog catalog(messages);
    for(const auto& binding:layoutTextMessageBindings(result.layout,"TooBad"))result.layout.panes[binding.first].text->text=catalog.plainText(binding.second);
    Rarc fonts(assets.readFile("/LayoutData/Font.arc"));for(auto name:result.layout.fontNames){std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});auto data=fonts.readFile("/"+name);result.fonts.push_back(decodeResourceFont(BinaryView(data)));}
    for(const char* name:{"Appear","End","Wait"}){std::string resource=name;std::transform(resource.begin(),resource.end(),resource.begin(),[](unsigned char c){return char(std::tolower(c));});auto data=archive.readFile("/anim/"+resource+".brlan");result.animations.emplace(name,decodeLayoutAnimation(BinaryView(data)));}return result;
}

