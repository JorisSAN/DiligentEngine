#include "Player.h"
#include "Log.h"

namespace Diligent
{

/*Player::Player()
{
}
*/

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

void Player::Initialize(float3 spawnPosition, Quaternion spawnRotation, ReactPhysic* reactPhysic, float cameraRotationSpeed, float cameraMoveSpeed)
{
    //Player
    setPosition(spawnPosition);
    setRotation(spawnRotation);

    //Rigidbody and collider
    reactphysics3d::Vector3 rbPos = reactphysics3d::Vector3(spawnPosition.x, spawnPosition.y, spawnPosition.z);
    reactphysics3d::Quaternion rbRot = reactphysics3d::Quaternion(spawnRotation.get_q().x, spawnRotation.get_q().y, spawnRotation.get_q().z, spawnRotation.get_q().a);
    reactphysics3d::Transform rbTrans(rbPos, rbRot);
    RigidbodyComponent* rb = new RigidbodyComponent(GetActor(), rbTrans, reactPhysic->GetPhysicWorld());
    addComponent(rb);

    CapsuleShape* capsuleShape = reactPhysic->GetPhysicCommon()->createCapsuleShape(1,1.8);
    CollisionComponent* colComp = new CollisionComponent(GetActor(), capsuleShape);
    colComp->SetCollider(rb->GetRigidBody()->addCollider(capsuleShape, rbTrans));
    addComponent(colComp);

    //Camera
    float3 cameraPos = spawnPosition + float3(0, 0.5f, 0);
    m_Camera->SetPos(cameraPos);
    float4 cameraRot = spawnRotation.get_q();
    m_Camera->SetRotation(cameraRot.x, cameraRot.y, cameraRot.z);
    m_Camera->SetRotationSpeed(cameraRotationSpeed);
    m_Camera->SetMoveSpeed(cameraMoveSpeed);
    m_Camera->SetSpeedUpScales(5.f, 10.f);

}

void Player::UpdatePlayer(double CurrTime, double ElapsedTime, InputController& Controller)
{
    UpdatePosition(Controller);
    //Update Camera
    

    //Update GLTF
    GLTFObject::UpdateActor(CurrTime, ElapsedTime);
}

void Player::UpdatePosition(InputController& Controller)
{
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

    if (Controller.IsKeyDown(InputKeys::MoveUp))
        MoveDirection.y += 1.0f;
    if (Controller.IsKeyDown(InputKeys::MoveDown))
        MoveDirection.y -= 1.0f;


    string message = "Move direction = " + std::to_string(MoveDirection.x) + "," + std::to_string(MoveDirection.y) + "," + std::to_string(MoveDirection.z);
    Diligent::Log::Instance().addInfo(message);
    Diligent::Log::Instance().Draw();
}

void Player::UpdatePRotation(InputController& Controller)
{
}


}

