#pragma once
#include <Native/ExtraModelMatrices.hpp>
// Exercise the actual matrix binding used by Mario's shadow/silhouette packet.
inline void verifyRetailExtraModelMatrices(WiiAssets& disc,JKRHeap& parent){
    struct StateGuard{
        void* pos=j3dSys.getVtxPos();void* nrm=j3dSys.getVtxNrm();GXColor* color=static_cast<GXColor*>(j3dSys.getVtxCol());
        J3DModel* model=j3dSys.getModel();J3DShapePacket* packet=j3dSys.getShapePacket();
        Mtx* draw=j3dSys.mModelDrawMtx;Mtx33* normal=j3dSys.mModelNrmMtx;
        ~StateGuard(){j3dSys.setVtxPos(pos);j3dSys.setVtxNrm(nrm);j3dSys.setVtxCol(color);j3dSys.setModel(model);j3dSys.setShapePacket(packet);j3dSys.mModelDrawMtx=draw;j3dSys.mModelNrmMtx=normal;}
    } stateGuard;
    Native::GDCurrentScope writerGuard;u8 commands[256]{};GDLObj writer;
    GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);
    auto* oldStyle=new(std::align_val_t{32}) Mtx[64];bool reproduced=false;
    try{j3dSys.setModelDrawMtx(oldStyle);}catch(const std::invalid_argument& e){reproduced=std::strstr(e.what(),"not registered")!=nullptr;}
    ::operator delete[](oldStyle,std::align_val_t{32});assert(reproduced);
    std::puts("PASS reproduced old player allocation failure through original J3DSys/GX_POS_MTX_ARRAY binding");
    auto* owner=JKRExpHeap::create(256*1024,&parent,false);assert(owner);owner->becomeCurrentHeap();
    std::array<Mtx*,8> views{};std::array<u32,8> addresses{};
    auto freeBefore=owner->getTotalFreeSize();bool zero=false,overflow=false;
    try{Native::allocateExtraModelMatrices(0);}catch(const std::invalid_argument&){zero=true;}
    try{Native::allocateExtraModelMatrices(std::numeric_limits<size_t>::max());}catch(const std::bad_array_new_length&){overflow=true;}
    assert(zero&&overflow&&owner->getTotalFreeSize()==freeBefore);
    for(unsigned view=0;view<views.size();++view){
        views[view]=Native::allocateExtraModelMatrices(64);assert((uintptr_t(views[view])&31)==0);
        assert(JKRHeap::findFromRoot(views[view])==owner&&Native::hasHeapGpuRange(views[view]));
        addresses[view]=Native::gpuResources().byteAddress(views[view]);
        for(unsigned m=0;m<64;++m)for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col)
            views[view][m][row][col]=float(int(view)*100+int(m)*12+int(row)*4+int(col))-7.25f;
        DCStoreRangeNoSync(views[view],64*sizeof(Mtx));
        auto* bytes=Native::gpuResources().resolve(addresses[view],64*sizeof(Mtx));
        for(unsigned i=0;i<64*12;++i){u32 bits;std::memcpy(&bits,reinterpret_cast<float*>(views[view])+i,4);assert(be32(bytes+i*4)==bits);}
    }
    Rarc archive(disc.readFile("/ObjectData/MarioShadow.arc"));std::vector<uint8_t> source;
    for(const auto& f:archive.files)if(f.path.find(".bdl")!=std::string::npos||f.path.find(".bmd")!=std::string::npos){source=archive.readFile(f.path);break;}
    assert(!source.empty());
    {NativeModelInstance shadow(source,*owner);assert(shadow.data.getDrawMtxNum()>0&&shadow.data.getDrawMtxNum()<=64);
        auto* shape=shadow.model().getShapePacket(0)->getShape();
        auto* slots=shadow.matrices->buffer.getDrawMtxPtrPtr();Mtx* original=slots[0];
        for(unsigned view=0;view<views.size();++view){
            // Same borrowed main-model view pointer that Mario assigns to its shadow.
            slots[0]=views[view];GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);
            shadow.model().getShapePacket(0)->prepareDraw();shape->setArrayAndBindPipeline();
            Native::GxCommandState decoded;decoded.consume(commands,GDGetCurrOffset(),&Native::gpuResources());
            const unsigned index=GX_POS_MTX_ARRAY-GX_VA_POS;
            assert(decoded.cp(0xa0+index)==addresses[view]&&decoded.cp(0xb0+index)==sizeof(Mtx));
            assert(j3dSys.mModelDrawMtx==views[view]);
        }
        slots[0]=original;views[1][0][0][0]=-123.25f;DCStoreRangeNoSync(views[1],sizeof(Mtx));
        u32 expected;std::memcpy(&expected,&views[1][0][0][0],4);
        assert(be32(Native::gpuResources().resolve(addresses[1],4))==expected);
        std::printf("PASS retail MarioShadow original shape/packet matrix binding: %u authored matrices, eight shared player views, float byte order and cache-flush updates\n",unsigned(shadow.data.getDrawMtxNum()));
    }
    parent.becomeCurrentHeap();JKRHeap::destroy(owner);
    for(unsigned i=0;i<views.size();++i){assert(!Native::hasHeapGpuRange(views[i]));bool removed=false;try{Native::gpuResources().resolve(addresses[i],1);}catch(const std::out_of_range&){removed=true;}assert(removed);}
    std::puts("PASS extra player matrix ownership: alignment, bounds, shared aliases and owning-heap teardown unregister all views");
}