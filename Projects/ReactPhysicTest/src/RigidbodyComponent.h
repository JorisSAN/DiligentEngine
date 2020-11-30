#pragma once
#include "Component.h"
#include <ReactPhysic.h>

namespace Diligent
{

class RigidbodyComponent : public Component
{
public:
    RigidbodyComponent(Actor* ownerP);
    RigidbodyComponent(Actor* ownerP, int updateOrder);
    RigidbodyComponent(Actor* ownerP, Transform transform, PhysicsWorld* _world);
    RigidbodyComponent(Actor* ownerP, Transform transform, PhysicsWorld* _world, int updateOrder);
    RigidbodyComponent() = delete;
    virtual ~RigidbodyComponent();
    RigidbodyComponent(const RigidbodyComponent&) = delete;
    RigidbodyComponent & operator=(const RigidbodyComponent&) = delete;

    virtual void update(double CurrTime, double ElapsedTime);

    //Getter / Setters
    void SetRigidBody(RigidBody* rigidbody);
    RigidBody* GetRigidBody();

private: 
    RigidBody* _rigidBody;
};

}//namespace Diligent