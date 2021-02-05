#pragma once
#include "Component.h"
#include "ReactPhysic.hpp"
#include <reactphysics3d/collision/shapes/CollisionShape.h>

namespace Diligent
{

class CollisionComponent : public Component
{
public:

    CollisionComponent(Actor* ownerP, CollisionShape* collisionShape);
    CollisionComponent(Actor* ownerP, CollisionShape* collisionShape, int updateOrder);
    CollisionComponent() = delete;
    virtual ~CollisionComponent();
    CollisionComponent(const CollisionComponent&) = delete;
    CollisionComponent& operator=(const CollisionComponent&) = delete;

    virtual void update(double CurrTime, double ElapsedTime);

    TypeID GetType() const override { return TCollisionComponent; }

    //Getter / Setters
    CollisionShape* GetCollisionShape() { return _collisionShape; }
    void SetCollisionShape(CollisionShape* collisionShape) { _collisionShape =collisionShape; }

    void SetLocalTransform(reactphysics3d::Transform vector) { _collider->setLocalToBodyTransform(vector); }

    void SetCollider(Collider* collider) { _collider = collider; }
    Collider* GetCollider() { return _collider; }

private:
    CollisionShape* _collisionShape;
    Collider*                    _collider;
}; 

} //namespace Diligent