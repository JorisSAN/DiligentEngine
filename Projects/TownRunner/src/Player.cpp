#include "Player.h"
#include "Log.h"

namespace Diligent
{

Player::Player(const SampleInitInfo& InitInfo, BackgroundMode backGroundP)
{
    GLTFObject::Initialize(InitInfo);
    m_BackgroundMode = backGroundP;
}

Player::Player(const SampleInitInfo& InitInfo, BackgroundMode backGroundP, std::string name)
{
    GLTFObject::Initialize(InitInfo);
    m_BackgroundMode = backGroundP;
    _actorName       = name;
}

void Player::UpdatePlayer(double CurrTime, double ElapsedTime, InputController& Controller)
{
    //Update Camera
    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));

    //Update GLTF
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);   
}


}

