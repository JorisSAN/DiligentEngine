#include "ReactPhysic.hpp"
#include <reactphysics3d/engine/Timer.h>

// ReactPhysics3D namespace
using namespace reactphysics3d;

ReactPhysic::ReactPhysic()
{
    // Create the world settings
    PhysicsWorld::WorldSettings settings;
    settings.gravity = Vector3(0, -1.5, 0);

    // Create the physics world with your settings
    _world = _physicsCommon.createPhysicsWorld(settings);
    //_world = _physicsCommon.createPhysicsWorld();
}

ReactPhysic::~ReactPhysic()
{
}

void ReactPhysic::Update()
{
    _world->update(_timeStep);
}
