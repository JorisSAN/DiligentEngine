#pragma once
#include "GLTFObject.h"
#include "Camera.h"

namespace Diligent
{

class Player : public GLTFObject
{
public:
    Player(const SampleInitInfo& InitInfo, BackgroundMode backGround);

    Player(const SampleInitInfo& InitInfo, BackgroundMode backGround, std::string name);

    void UpdatePlayer(double CurrTime, double ElapsedTime, InputController& Controller);

    Camera* GetCamera() { return &m_Camera; }

private:
    Camera m_Camera;
};

}