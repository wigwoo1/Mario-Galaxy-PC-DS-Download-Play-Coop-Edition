#include "Game/Util/ModelUtil.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
namespace MR {
    XanimePlayer* newXanimePlayer(const ResourceHolder* pModelResourceHolder, const char* pChar, const ResourceHolder* pUnused, J3DMdlFlag mdlFlag,
                                  XanimeResourceTable* pAnimeResourceTable) {
        J3DModel* newModel = newJ3DModel(pModelResourceHolder, pChar, mdlFlag);
        return new XanimePlayer(newModel, pAnimeResourceTable);
    }

    XanimeResourceTable* newXanimeResourceTable(ResourceHolder* pResourceHolder) {
        return new XanimeResourceTable(pResourceHolder);
    }


}
