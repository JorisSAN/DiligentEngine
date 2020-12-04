#include "Helmet.h"

namespace Diligent
{

Helmet::Helmet(const SampleInitInfo& InitInfo, BackgroundMode backGroundP)
{
    GLTFObject::Initialize(InitInfo);
    setObjectPath("models/DamagedHelmet/DamagedHelmet.gltf");
    m_BackgroundMode = backGroundP;
}


Helmet::Helmet(const SampleInitInfo& InitInfo, BackgroundMode backGround, std::string name)
{
    GLTFObject::Initialize(InitInfo);
    setObjectPath("models/DamagedHelmet/DamagedHelmet.gltf");
    m_BackgroundMode = backGround;
    _actorName       = name;
}


void Helmet::UpdateActor(double CurrTime, double ElapsedTime)
{
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);

    //setPosition(float3(0.0f, cos(static_cast<float>(CurrTime) * 1.0f), sin(static_cast<float>(CurrTime) * 1.0f)));
    //setRotation(Quaternion::concatenate(getRotation(), Quaternion::RotationFromAxisAngle(float3(0, 1, 0), 0.01f)));
}

} // namespace Diligent