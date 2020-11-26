#include "RigidbodyComponent.h"

namespace Diligent
{

RigidbodyComponent::RigidbodyComponent(Actor* ownerP) :
    Component(ownerP) 
{
    _rigidBody = nullptr;
}

RigidbodyComponent::RigidbodyComponent(Actor* ownerP, int updateOrder) : 
    Component(ownerP, updateOrder)
{
    _rigidBody = nullptr;
}

RigidbodyComponent::RigidbodyComponent(Actor* ownerP, Transform transform, PhysicsWorld* _world) :
    Component(ownerP)
{
    _rigidBody = _world->createRigidBody(transform);
}

RigidbodyComponent::RigidbodyComponent(Actor* ownerP, Transform transform, PhysicsWorld* _world, int updateOrder) :
    Component(ownerP, updateOrder)
{
    _rigidBody = _world->createRigidBody(transform);
}


RigidbodyComponent::~RigidbodyComponent()
{
}


void RigidbodyComponent::update(double CurrTime, double ElapsedTime)
{
}


//Getter and setter
void RigidbodyComponent::SetRigidBody(RigidBody* rigidbody)
{
    _rigidBody = rigidbody;
}


RigidBody* RigidbodyComponent::GetRigidBody() 
{
    return _rigidBody;
}

} // namespace Diligent