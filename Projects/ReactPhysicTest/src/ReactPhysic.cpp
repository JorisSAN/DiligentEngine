#include "ReactPhysic.h"

// ReactPhysics3D namespace
using namespace reactphysics3d;

ReactPhysic::ReactPhysic()
{
    _world = _physicsCommon.createPhysicsWorld();
}

ReactPhysic::~ReactPhysic()
{
}

void ReactPhysic::Update()
{
    _world->update(timeStep);

    std::ofstream myfile;
    myfile.open("example.txt", std::ofstream::out | std::ofstream::app);
    
    // Get the updated position of each bodies
    for (RigidBody* body : _rigidbodyList)
        {
        const Transform& transform = body->getTransform();
        const Vector3&   position  = transform.getPosition();

        // Display the position of the body
        myfile << "Body Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
        }


    myfile.close();
}

void ReactPhysic::AddRigidbody(Vector3 position, Quaternion orientation)
{
    // Create a rigid body in the world
    Transform  transform(position, orientation);
    RigidBody* body = _world->createRigidBody(transform);
    _rigidbodyList.push_back(body);
}
