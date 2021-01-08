#include "CollisionComponent.hpp"
#include "Log.h"


namespace Diligent
{
CollisionComponent::CollisionComponent(Actor* ownerP, CollisionShape* collisionShape) :
    Component(ownerP)
{
    _collisionShape.emplace_back(collisionShape);
    _collider = nullptr;
}

CollisionComponent::CollisionComponent(Actor* ownerP, CollisionShape* collisionShape, int updateOrder) :
    Component(ownerP, updateOrder)
{
    _collisionShape.emplace_back(collisionShape);
    _collider = nullptr;
}

CollisionComponent::CollisionComponent(Actor* ownerP, std::vector<CollisionShape*> collisionShape) :
    Component(ownerP) 
{
    _collisionShape = collisionShape;
}

CollisionComponent::CollisionComponent(Actor* ownerP, std::vector<CollisionShape*> collisionShape, int updateOrder) :
        Component(ownerP, updateOrder) 
    {
        _collisionShape = collisionShape;
    }

CollisionComponent::~CollisionComponent() {}

void CollisionComponent::update(double CurrTime, double ElapsedTime)
{
    //reactphysics3d::Transform localToBodyTransform = _collider->getLocalToBodyTransform();
    //string message = "Local to body position, x = " + std::to_string(localToBodyTransform.getPosition().x) + " , y = " 
    //    + std::to_string(localToBodyTransform.getPosition().y) + "  , z = " + std::to_string(localToBodyTransform.getPosition().z);

    //Diligent::Log::Instance().addInfo(message);
    //Diligent::Log::Instance().Draw();
}

}