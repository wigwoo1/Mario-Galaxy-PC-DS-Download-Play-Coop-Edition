#pragma once
#include "native_joint_tables.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/J3DGraphAnimator/J3DJointTree.hpp>
// Retained model resource owns copied joints and the data borrowed by JUTNameTab.
// Keep alive while the original joint tree references nodes, pointer table or names.
struct NativeJointResource {
    NativeJointTables tables;
    std::vector<std::unique_ptr<J3DJoint>> nodes;
    std::vector<J3DJoint*> pointers;
    std::unique_ptr<JUTNameTab> names;
    J3DJoint* root=nullptr;
    explicit NativeJointResource(BinaryView block):tables(block){
        const auto* native=tables.loaderBlock();J3DJointFactory factory(*native);
        if(native->mpNameTable)names=std::make_unique<JUTNameTab>(reinterpret_cast<const ResNTAB*>(reinterpret_cast<const uint8_t*>(native)+reinterpret_cast<uintptr_t>(native->mpNameTable)));
        nodes.reserve(native->mJointNum);pointers.reserve(native->mJointNum);
        for(u16 i=0;i<native->mJointNum;++i){nodes.emplace_back(factory.create(i));pointers.push_back(nodes.back().get());}
    }
    NativeJointResource(const NativeJointResource&)=delete;
    NativeJointResource& operator=(const NativeJointResource&)=delete;
    void buildHierarchy(BinaryView info){
        assetCheck(info.size()>=24 && info.tag(0)=="INF1" && info.u32(4)==info.size(),"Invalid INF1 block");
        size_t start=info.u32(20);assetCheck(start>=24 && start<=info.size(),"Invalid hierarchy offset");
        std::vector<int> parents(nodes.size(),-1),order,stack;std::vector<bool> seen(nodes.size());int parent=-1,last=-1;bool ended=false;
        for(size_t p=start;p+4<=info.size();p+=4){u16 type=info.u16(p),index=info.u16(p+2);
            if(type==0){assetCheck(stack.empty(),"Unbalanced joint hierarchy");ended=true;break;}
            if(type==1){assetCheck(last>=0 && stack.size()<128,"Invalid hierarchy open");stack.push_back(parent);parent=last;}
            else if(type==2){assetCheck(!stack.empty(),"Hierarchy stack underflow");parent=stack.back();stack.pop_back();last=parent;}
            else if(type==16){assetCheck(index<nodes.size() && !seen[index],"Invalid hierarchy joint");seen[index]=true;parents[index]=parent;order.push_back(index);last=index;}
            else assetCheck(type==17 || type==18,"Unknown hierarchy command");
        }
        assetCheck(ended && order.size()==nodes.size(),"Incomplete joint hierarchy");
        root=nullptr;for(auto& node:nodes){node->mChild=nullptr;node->mYounger=nullptr;}
        for(int index:order){int pi=parents[index];J3DJoint** link=pi<0?&root:&nodes[pi]->mChild;
            while(*link)link=&(*link)->mYounger;*link=nodes[index].get();}
    }
    void attachMeshes(BinaryView info,const std::vector<J3DMaterial*>& materials,const std::vector<J3DShape*>& shapes){
        assetCheck(info.size()>=24 && info.tag(0)=="INF1" && info.u32(4)==info.size(),"Invalid mesh hierarchy");
        size_t start=info.u32(20);assetCheck(start>=24 && start<=info.size(),"Invalid mesh hierarchy offset");
        struct Attachment{int joint;u16 index;bool shape;};std::vector<Attachment> pending;
        std::vector<int> stack;int parent=-1,current=-1;bool ended=false;std::vector<int> mesh(nodes.size(),-1);
        for(size_t p=start;p+4<=info.size();p+=4){u16 type=info.u16(p),index=info.u16(p+2);
            if(type==0){assetCheck(stack.empty(),"Unbalanced mesh hierarchy");ended=true;break;}
            if(type==1){assetCheck(current>=0 && stack.size()<128,"Invalid mesh hierarchy open");stack.push_back(parent);parent=current;}
            else if(type==2){assetCheck(!stack.empty(),"Mesh hierarchy underflow");parent=stack.back();stack.pop_back();current=parent;}
            else if(type==16){assetCheck(index<nodes.size(),"Invalid mesh joint");current=index;}
            else if(type==17){assetCheck(parent>=0 && index<materials.size() && materials[index],"Invalid hierarchy material");mesh[parent]=index;pending.push_back({parent,index,false});}
            else if(type==18){assetCheck(parent>=0 && mesh[parent]>=0 && index<shapes.size() && shapes[index],"Invalid hierarchy shape");pending.push_back({parent,index,true});}
            else assetCheck(false,"Unknown mesh hierarchy command");
        }
        assetCheck(ended,"Missing mesh hierarchy terminator");
        for(auto attachment:pending){auto* joint=nodes[attachment.joint].get();
            if(!attachment.shape){auto* material=materials[attachment.index];joint->addMesh(material);material->setJoint(joint);}
            else{auto* material=joint->getMesh();auto* shape=shapes[attachment.index];material->addShape(shape);shape->setMaterial(material);}
        }
    }
    void bind(J3DJointTree& tree){tree.mJointNum=u16(pointers.size());tree.mJointNodePointer=pointers.data();tree.mJointName=names.get();tree.mRootNode=root;}
};


