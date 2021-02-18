#include "AnimPeople.h"

namespace Diligent
{

AnimPeople::AnimPeople(const SampleInitInfo& InitInfo, BackgroundMode backGroundP, RefCntAutoPtr<IRenderPass>& RenderPass)
{
    GLTFObject::Initialize(InitInfo, RenderPass);
    setObjectPath("models/CesiumMan/CesiumMan.gltf");
    m_BackgroundMode = backGroundP;
    _actorType       = ActorType::AnimPeople;
}

void AnimPeople::UpdateActor(double CurrTime, double ElapsedTime)
{
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);
    setRotation(Quaternion::concatenate(getRotation(), Quaternion::RotationFromAxisAngle(float3(0, 1, 0), 0.01f)));
}

} // namespace Diligent