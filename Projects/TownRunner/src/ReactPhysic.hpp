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

    //Getter and setters
    PhysicsWorld* GetPhysicWorld() { return _world; }
    const decimal GetTimeStep() { return _timeStep; }

private:
    PhysicsCommon         _physicsCommon;
    PhysicsWorld*         _world;
    const decimal         _timeStep = 1.0f / 60.0f;
};