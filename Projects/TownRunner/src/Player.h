#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include "GLTFObject.h"
#include "CameraPlayer.h"
#include "AdvancedMath.hpp"
#include "RigidbodyComponent.hpp"
#include "CollisionComponent.hpp"

namespace Diligent
{

class Player : public GLTFObject
{
public:
    Player();

    Player(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass);

    Player(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass, std::string name);

    void Initialize(float3 spawnPosition, Quaternion spawnRotation, ReactPhysic* _reactPhysic, float cameraRotationSpeed, float cameraMoveSpeed);

    void UpdatePlayer(double CurrTime, double ElapsedTime, InputController& Controller);

    void SetCamerat(CameraPlayer* c) { m_Camera = c; }
    CameraPlayer* GetCamera() { return m_Camera; } 

private:

    void UpdatePosition(InputController& Controller);
    void UpdatePRotation(InputController& Controller);

    //Camera
    CameraPlayer* m_Camera;
};

}

#endif