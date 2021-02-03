#include "Plane.h"

namespace Diligent
{

Plane::Plane(const SampleInitInfo& InitInfo, BackgroundMode backGroundP, RefCntAutoPtr<IRenderPass>& RenderPass)
{
    GLTFObject::Initialize(InitInfo, RenderPass);
    setObjectPath("models/Plane/Plane.gltf");
    m_BackgroundMode = backGroundP;
}

void Plane::UpdateActor(double CurrTime, double ElapsedTime)
{
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);

    setScale(3);
}

} // namespace Diligent