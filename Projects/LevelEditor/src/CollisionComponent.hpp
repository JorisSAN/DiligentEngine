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
    CollisionComponent(Actor* ownerP, std::vector<CollisionShape*> collisionShape);
    CollisionComponent(Actor* ownerP, std::vector<CollisionShape*> collisionShape, int updateOrder);
    CollisionComponent() = delete;
    virtual ~CollisionComponent();
    CollisionComponent(const CollisionComponent&) = delete;
    CollisionComponent& operator=(const CollisionComponent&) = delete;

    virtual void update(double CurrTime, double ElapsedTime);

    TypeID GetType() const override { return TCollisionComponent; }

    //Getter / Setters
    void SetCollisionShape(std::vector<CollisionShape*> collisionShape) { _collisionShape = collisionShape; }
    std::vector<CollisionShape*> GetCollisionShape() { return _collisionShape; }
    void AddCollisionShape(CollisionShape* collisionShape) { _collisionShape.emplace_back(collisionShape); }
    void RemoveCollisionShape(CollisionShape* collisionShape) { _collisionShape.push_back(collisionShape); }

    void SetCollider(Collider* collider) { _collider = collider; }
    Collider* GetCollider() { return _collider; }

private:
    std::vector<CollisionShape*> _collisionShape;
    Collider*                    _collider;
};

}

//namespace Diligent