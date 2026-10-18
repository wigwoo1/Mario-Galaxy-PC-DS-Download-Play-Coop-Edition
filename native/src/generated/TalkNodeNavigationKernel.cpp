#include <Game/NPC/TalkNodeCtrl.hpp>
#include <Game/System/MessageHolder.hpp>
TalkNodeCtrl::TalkNodeCtrl() : _0(), mCurrentNodeIdx(-1), mMessageInfo() {
    mHistory.mCount = 0;
    _38 = nullptr;
    mCurrentNode = nullptr;
    mNodeData = -1;
}

bool TalkNodeCtrl::isExistNextNode() const {
    return getNextNode() != nullptr;
}

bool TalkNodeCtrl::isNextNodeMessage() const {
    TalkNode* next = getNextNode();

    if (next == nullptr || next->mNodeType != 1) {
        return false;
    }

    return true;
}

bool TalkNodeCtrl::isCurrentNodeEvent() const {
    return getCurrentNodeEvent() != nullptr;
}

TalkNode* TalkNodeCtrl::getNextNode() const {
    TalkNode* node = mCurrentNode;
    u16 idx;

    if (node == nullptr) {
        return nullptr;
    }

    if (node->mNodeType == 1) {
        idx = node->mNextIdx;
        if (idx != 0xFFFF) {
            return MessageSystem::getSceneMessageData()->getNode(idx);
        }
    } else if (node->mNodeType == 3) {
        idx = node->mIndex;
        MessageData* msg = MessageSystem::getSceneMessageData();

        if (msg->isValidBranchNode(idx)) {
            idx = node->mIndex;
            MessageData* msg = MessageSystem::getSceneMessageData();
            return msg->getBranchNode(idx);
        }
    }

    return nullptr;
}

TalkNode* TalkNodeCtrl::getNextNodeBranch() const {
    TalkNode* node = getNextNode();

    if (node == nullptr) {
        return nullptr;
    }

    return node->mNodeType != 2 ? nullptr : node;
}

TalkNode* TalkNodeCtrl::getCurrentNodeBranch() const {
    if (mCurrentNode != nullptr && mCurrentNode->mNodeType == 2) {
        return mCurrentNode;
    }

    return nullptr;
}

TalkNode* TalkNodeCtrl::getCurrentNodeMessage() const {
    if (mCurrentNode != nullptr && mCurrentNode->mNodeType == 1) {
        return mCurrentNode;
    }

    return nullptr;
}

TalkNode* TalkNodeCtrl::getCurrentNodeEvent() const {
    if (mCurrentNode != nullptr && mCurrentNode->mNodeType == 3) {
        return mCurrentNode;
    }

    return nullptr;
}

TalkNode* TalkNodeCtrl::getNextNodeEvent() const {
    TalkNode* node = getNextNode();

    if (node == nullptr) {
        return nullptr;
    }

    return node->mNodeType != 3 ? nullptr : node;
}

