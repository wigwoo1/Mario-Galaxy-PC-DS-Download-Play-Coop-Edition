#pragma once
#include "layout_resource.hpp"
// Native adaptation of LayoutManager::initTextBoxRecursive and MR::getLayoutMessageID.
inline std::vector<std::pair<size_t,std::string>> layoutTextMessageBindings(const NativeLayoutResource& layout,const std::string& layoutName){
    assetCheck(!layoutName.empty(),"Empty localized layout name");std::vector<std::string> inherited;std::vector<std::pair<size_t,std::string>> result;
    for(size_t i=0;i<layout.panes.size();++i){const auto& pane=layout.panes[i];assetCheck(pane.parent>=-1&&(pane.parent<0||size_t(pane.parent)<i),"Invalid localized layout parent");std::string key=pane.userData;if(key.empty()&&pane.parent>=0)key=inherited[size_t(pane.parent)];inherited.push_back(key);if(pane.text&&!key.empty()){auto id="Layout_"+layoutName+key;assetCheck(id.size()<256,"Layout message ID exceeds original buffer");result.emplace_back(i,std::move(id));}}return result;
}
inline void layoutTextBindingTests(){
    NativeLayoutResource layout;NativeLayoutPane root;root.userData="Root";layout.panes.push_back(root);NativeLayoutPane child;child.parent=0;child.text=NativeLayoutText{};layout.panes.push_back(child);NativeLayoutPane other=child;other.userData="Own";layout.panes.push_back(other);auto bindings=layoutTextMessageBindings(layout,"Test");assetCheck(bindings==std::vector<std::pair<size_t,std::string>>{{1,"Layout_TestRoot"},{2,"Layout_TestOwn"}},"Inherited layout message binding mismatch");layout.panes[0].userData.clear();bindings=layoutTextMessageBindings(layout,"Test");assetCheck(bindings==std::vector<std::pair<size_t,std::string>>{{2,"Layout_TestOwn"}},"Unbound layout text received an ID");
}
