#pragma once

#include <reactphysics3d/reactphysics3d.h>
#include <iostream> 
#include <list>

// ReactPhysics3D namespace
using namespace reactphysics3d;

class ReactPhysic
{
public:
    ReactPhysic();
    ~ReactPhysic();
    void Update();
    void AddRigidbody(Vector3 position, Quaternion orientation);


private:
    PhysicsCommon _physicsCommon;
    PhysicsWorld* _world;
    std::list<RigidBody*> _rigidbodyList;
    const decimal         timeStep = 1.0f / 60.0f;
};