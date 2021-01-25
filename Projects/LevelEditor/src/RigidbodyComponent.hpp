#pragma once
#include "Component.h"
#include "ReactPhysic.hpp"

namespace Diligent
{

class RigidbodyComponent : public Component
{
public:
    RigidbodyComponent(Actor* ownerP, Transform transform, PhysicsWorld* _world);
    RigidbodyComponent(Actor* ownerP, Transform transform, PhysicsWorld* _world, int updateOrder);
    RigidbodyComponent() = delete;
    virtual ~RigidbodyComponent();
    RigidbodyComponent(const RigidbodyComponent&) = delete;
    RigidbodyComponent& operator=(const RigidbodyComponent&) = delete;


    virtual void update(double CurrTime, double ElapsedTime);

    TypeID GetType() const override { return TRigidbodyComponent; }

    //Getter / Setters
    void       SetRigidBody(RigidBody* rigidbody) { _rigidBody = rigidbody; }
    RigidBody* GetRigidBody() { return _rigidBody; }

private:
    RigidBody* _rigidBody;
};

} //namespace Diligent