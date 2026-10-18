// Native profile reload observer. This diagnoses original state only.
    if(const char* expected=std::getenv("GALAXY_DIAGNOSTIC_EXPECT_PROFILE")){
        NativeDiagnosticInput::observe(NativeDiagnosticInput::Phase::Quiet);
        if(expected[0]<'1'||expected[0]>'6'||expected[1])throw std::runtime_error("Reload diagnostic expects one profile number from1 to6");
        if(std::getenv("GALAXY_DIAGNOSTIC_SELECT_EMPTY_SLOT")||std::getenv("GALAXY_DIAGNOSTIC_SELECT_SAVED_SLOT")||std::getenv("GALAXY_DIAGNOSTIC_CREATE_PROFILE"))throw std::runtime_error("Reload observation cannot select or create a profile");
        const int id=expected[0]-'0';
        if(!mItems||!mUserFile)throw std::runtime_error("Reload observation lacks original file-selector data");
        FileSelectItem* target=nullptr;
        for(int i=0;i<mItems->getObjNum();++i){
            auto* item=static_cast<FileSelectItem*>(mItems->getActor(i));
            if(!item||item->_140<1||item->_140>6)throw std::runtime_error("Reload observation has an invalid original slot ID");
            if(item->_140==id){if(target)throw std::runtime_error("Reload observation found a duplicate original slot ID");target=item;}
            if(MR::isFirstStep(this)){char line[200];std::snprintf(line,sizeof(line),"Diagnostic reloaded slot=%d created=%u isNew=%u isExist=%u",item->_140,unsigned(mUserFile[item->_140-1].isCreated()),unsigned(item->isNew()),unsigned(item->isExist()));galaxyNativeBootTrace(line);}
        }
        const bool created=mUserFile[id-1].isCreated(),isNew=target&&target->isNew(),exists=target&&target->isExist();
        if(MR::isFirstStep(this)){char line[200];std::snprintf(line,sizeof(line),"Diagnostic expected persisted profile=%d created=%u isNew=%u isExist=%u",id,unsigned(created),unsigned(isNew),unsigned(exists));galaxyNativeBootTrace(line);}
        if(!target||!created||isNew||!exists)throw std::runtime_error("Original game did not recognize the expected persisted profile");
    }
