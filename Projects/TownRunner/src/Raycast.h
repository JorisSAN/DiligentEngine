#pragma once
#include "ReactPhysic.hpp"
#include "MyRaycastCallback.h"

using namespace reactphysics3d;

class Raycast
{
public:
    Raycast();
    Raycast(Vector3 startPoint, Vector3 EndPoint);
    ~Raycast();

    //Getter and setters
    Ray            GetRay() { return _ray; }
    void SetRaycastPosition(Vector3 startPoint, Vector3 EndPoint);
    CollisionBody* GetRaycastInfoCollisionBody() { return _raycastInfo.body; }
    Collider* GetRaycastInfoCollider() { return _raycastInfo.collider; }
    decimal GetRaycastInfoHitFraction() { return _raycastInfo.hitFraction; }
    int GetRaycastInfoMeshSubpart() { return _raycastInfo.meshSubpart; }
    int GetRaycastInfoTriangleIndex() { return _raycastInfo.triangleIndex; }
    Vector3 GetRaycastInfoVectorNormal() { return _raycastInfo.worldNormal; }
    Vector3 GetRaycastInfoWorldPoint() { return _raycastInfo.worldPoint; }

    ///Different way of using a raycast
    //Raycast against multiple bodies
    void UseRaycast(PhysicsWorld* world, MyRaycastCallback raycastCallback);
    //Raycast against a specific collider body or rigidbody, return true if the collision happend
    bool UseRaycastBody(CollisionBody* body);
    //Raycast against a specific collider, return true if the collision happend
    bool UseRaycastCollider(Collider* collider);

private:
    Vector3 _startPoint;
    Vector3 _endPoint;
    Ray     _ray;

    //raycast result
    RaycastInfo _raycastInfo;
};