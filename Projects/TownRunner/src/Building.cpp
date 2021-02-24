#include "Building.h"

Diligent::Building::Building(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass, std::string name, const char* path)
{
    GLTFObject::Initialize(InitInfo, RenderPass);
    _actorType = ActorType::Building;
    _actorName = name;
    m_BackgroundMode = backGround;
    setObjectPath(path);
    pathName = path;
}

void Diligent::Building::UpdatePlayer(double CurrTime, double ElapsedTime, InputController& Controller)
{
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);
    GLTFObject::computeWorldTransform();
}
