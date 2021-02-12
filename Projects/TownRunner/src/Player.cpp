#include "Player.h"
#include "Log.h"

namespace Diligent
{

Player::Player(const SampleInitInfo& InitInfo, BackgroundMode backGroundP, RefCntAutoPtr<IRenderPass>& RenderPass)
{
    GLTFObject::Initialize(InitInfo, RenderPass);
    setObjectPath("models/Cube/Cube.gltf");
    m_BackgroundMode = backGroundP;
}

Player::Player(const SampleInitInfo& InitInfo, BackgroundMode backGroundP, RefCntAutoPtr<IRenderPass>& RenderPass, std::string name)
{
    GLTFObject::Initialize(InitInfo, RenderPass);
    setObjectPath("models/Cube/Cube.gltf");
    m_BackgroundMode = backGroundP;
    _actorName       = name;
}

void Player::Initialize(float3 spawnPosition, Quaternion spawnRotation, ReactPhysic* reactPhysic, float3 cameraSpring, float capsuleRadius, float capsuleHeight, float cameraRotationSpeed, float cameraMoveSpeed, float jumpHeight)
{
    //Player
    setPosition(spawnPosition);
    setRotation(spawnRotation);

    
    //Rigidbody and collider
    reactphysics3d::Vector3 rbPos = reactphysics3d::Vector3(spawnPosition.x, spawnPosition.y, spawnPosition.z);
    reactphysics3d::Quaternion rbRot = reactphysics3d::Quaternion(spawnRotation.get_q().x, spawnRotation.get_q().y, spawnRotation.get_q().z, spawnRotation.get_q().a);
    reactphysics3d::Transform rbTrans(rbPos, rbRot);
    RigidbodyComponent* rb = new RigidbodyComponent(GetActor(), rbTrans, reactPhysic->GetPhysicWorld());
    rb->GetRigidBody()->setType(BodyType::KINEMATIC);
    rb->GetRigidBody()->setMass(80);
    _playerRB = rb;
    addComponent(rb);
    
    CapsuleShape* capsuleShape = reactPhysic->GetPhysicCommon()->createCapsuleShape(capsuleRadius, capsuleHeight);
    CollisionComponent* colComp = new CollisionComponent(GetActor(), capsuleShape);
    colComp->SetCollider(rb->GetRigidBody()->addCollider(capsuleShape, rbTrans));
    _playerCC = colComp;
    addComponent(colComp);

    _jumpHeight = jumpHeight;

    //Camera
    m_Camera = new CameraPlayer();
    SetCameraSpring(cameraSpring);
    float3 cameraPos = spawnPosition + cameraSpring;
    m_Camera->SetPos(cameraPos);
    float4 cameraRot = spawnRotation.get_q();
    m_Camera->SetRotation(cameraRot.x, cameraRot.y, cameraRot.z);
    m_Camera->SetRotationSpeed(cameraRotationSpeed);
    m_Camera->SetMoveSpeed(cameraMoveSpeed);
    m_Camera->SetSpeedUpScales(5.f, 10.f);
}

void Player::UpdatePlayer(double CurrTime, double ElapsedTime, InputController& Controller)
{
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);

    //----------------------------
    //Position
    //Create vector for direction
    float3 MoveDirection = float3(0, 0, 0);
    // Update acceleration vector based on keyboard state
    if (Controller.IsKeyDown(InputKeys::MoveForward))
        MoveDirection.z += 1.0f;
    if (Controller.IsKeyDown(InputKeys::MoveBackward))
        MoveDirection.z -= 1.0f;

    if (Controller.IsKeyDown(InputKeys::MoveRight))
        MoveDirection.x += 1.0f;
    if (Controller.IsKeyDown(InputKeys::MoveLeft))
        MoveDirection.x -= 1.0f;

    // Normalize vector so if moving in 2 dirs (left & forward),
    // the camera doesn't move faster than if moving in 1 dir
    auto len = length(MoveDirection);
    if (len != 0.0)
        MoveDirection /= len;

    //Check if we're sprinting
    bool IsSpeedUpScale = Controller.IsKeyDown(InputKeys::ShiftDown);

    //Apply sprinting speed
    MoveDirection *= m_Camera->m_fMoveSpeed;
    if (IsSpeedUpScale) MoveDirection *= m_Camera->m_fSpeedUpScale;

    //Update camera current speed
    m_Camera->m_fCurrentSpeed = length(MoveDirection);

    float3 PosDelta = MoveDirection * ElapsedTime;


    //----------------------------
    //Rotation
    {
        const auto& mouseState = Controller.GetMouseState();

        float MouseDeltaX = 0;
        float MouseDeltaY = 0;
        if (m_Camera->m_LastMouseState.PosX >= 0 && m_Camera->m_LastMouseState.PosY >= 0 &&
            m_Camera->m_LastMouseState.ButtonFlags != MouseState::BUTTON_FLAG_NONE)
        {
            MouseDeltaX = mouseState.PosX - m_Camera->m_LastMouseState.PosX;
            MouseDeltaY = mouseState.PosY - m_Camera->m_LastMouseState.PosY;
        }
        m_Camera->m_LastMouseState = mouseState;

        float fYawDelta   = MouseDeltaX * m_Camera->m_fRotationSpeed;
        float fPitchDelta = MouseDeltaY * m_Camera->m_fRotationSpeed;
        if (mouseState.ButtonFlags & MouseState::BUTTON_FLAG_LEFT)
        {
            m_Camera->m_fYawAngle += fYawDelta * -m_Camera->m_fHandness;
            m_Camera->m_fPitchAngle += fPitchDelta * -m_Camera->m_fHandness;
            m_Camera->m_fPitchAngle = std::max(m_Camera->m_fPitchAngle, -PI_F / 2.f);
            m_Camera->m_fPitchAngle = std::min(m_Camera->m_fPitchAngle, +PI_F / 2.f);
            rotation = Quaternion::RotationFromAxisAngle(float3(1, 0, 0), m_Camera->m_fPitchAngle) *
                Quaternion::RotationFromAxisAngle(float3(0, 1, 0), m_Camera->m_fYawAngle) *
                Quaternion::RotationFromAxisAngle(float3(0, 0, 1), m_Camera->m_fRollAngle);
        }
    }

    float4x4 ReferenceRotation = m_Camera->GetReferenceRotiation();

    float4x4 CameraRotation = Quaternion::createFromQuaternion(rotation) * ReferenceRotation;
    float4x4 WorldRotation  = CameraRotation.Transpose();

    float3 PosDeltaWorld = PosDelta * WorldRotation;

    //Update coords
    //Player
    reactphysics3d::Transform rbTrans = _playerRB->GetRigidBody()->getTransform();
    reactphysics3d::Vector3   rbPos   = rbTrans.getPosition();
    rbPos = reactphysics3d::Vector3(rbPos.x + PosDeltaWorld.x, rbPos.y, rbPos.z + PosDeltaWorld.z);
    rbTrans.setPosition(rbPos);
    _playerRB->GetRigidBody()->setTransform(rbTrans);
    //position += PosDeltaWorld; //player
    //Jump
    if (Controller.IsKeyDown(InputKeys::Jump) && _canJump)
    {
        _canJump       = false;
        string message = "JUMP";
        Diligent::Log::Instance().addInfo(message);
        Diligent::Log::Instance().Draw();
        reactphysics3d::Vector3 jump = reactphysics3d::Vector3(0, _jumpHeight, 0);
        _playerRB->GetRigidBody()->applyForceToCenterOfMass(jump);
    }
   
    LockColliderRotation();

    //Camera
    m_Camera->m_Pos = float3(rbPos.x, rbPos.y, rbPos.z) + _cameraSpring;
    m_Camera->m_ViewMatrix  = float4x4::Translation(-m_Camera->m_Pos) * CameraRotation;
    m_Camera->m_WorldMatrix = WorldRotation * float4x4::Translation(m_Camera->m_Pos);

    //Print message  
    /*string message = "Move direction = " + std::to_string(rbPos.x) + "," + std::to_string(rbPos.y) + "," + std::to_string(rbPos.z);
    Diligent::Log::Instance().addInfo(message);
    Diligent::Log::Instance().Draw();
    */
}

//We need to lock the collider rotation
void Player::LockColliderRotation()
{
    CollisionBody* body = _playerCC->GetCollider()->getBody();
    reactphysics3d::Transform bodyTrans = body->getTransform();
    bodyTrans.setOrientation(reactphysics3d::Quaternion::identity());
    body->setTransform(bodyTrans);
}

}

