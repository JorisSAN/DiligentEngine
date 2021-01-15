#include "Raycast.h"

Raycast::Raycast() :
    _startPoint(Vector3::zero()), _endPoint(Vector3::zero()), _ray(_startPoint, _endPoint)
{
}

Raycast::Raycast(Vector3 startPoint, Vector3 EndPoint) :
    _startPoint(startPoint), _endPoint(EndPoint), _ray(_startPoint, _endPoint)
{
    std::string message = "createtruc";

    Diligent::Log::Instance().addInfo(message);
    Diligent::Log::Instance().Draw();
}

Raycast::~Raycast()
{
}

void Raycast::SetRaycastPosition(Vector3 startPoint, Vector3 endPoint)
{
    _startPoint = startPoint;
    _endPoint   = endPoint;
    _ray = Ray(_startPoint, _endPoint);
}

void Raycast::UseRaycast(PhysicsWorld* world, MyRaycastCallback raycastCallback)
{
    std::string message = "isused";

    Diligent::Log::Instance().addInfo(message);
    Diligent::Log::Instance().Draw();
    world->raycast(_ray, &raycastCallback);
}

bool Raycast::UseRaycastBody(CollisionBody* body)
{
    return body->raycast(_ray, _raycastInfo);
}

bool Raycast::UseRaycastCollider(Collider* collider)
{
    return collider->raycast(_ray, _raycastInfo);
}
