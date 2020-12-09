#include "AnimPeople.h"

namespace Diligent
{

AnimPeople::AnimPeople(const SampleInitInfo& InitInfo, BackgroundMode backGroundP, RefCntAutoPtr<IRenderPass>& RenderPass)
{
    GLTFObject::Initialize(InitInfo, RenderPass);
    setObjectPath("models/CesiumMan/CesiumMan.gltf");
    m_BackgroundMode = backGroundP;
}

void AnimPeople::UpdateActor(double CurrTime, double ElapsedTime)
{
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);
    setRotation(Quaternion::RotationFromAxisAngle(float3(0, 1, 0), static_cast<float>(CurrTime) * 1.0f));
}

} // namespace Diligent