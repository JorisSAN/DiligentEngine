#include "MyRaycastCallback.h"
#include "RigidbodyComponent.hpp"
#include "Actor.h"

decimal MyRaycastCallback::notifyRaycastHit(const RaycastInfo& info)
{
    // Display the world hit point coordinates
    std::string message = "Hit point : " + std::to_string(info.worldPoint.x) + " - " + std::to_string(info.worldPoint.y) + " - " + std::to_string(info.worldPoint.z);

    Diligent::Log::Instance().addInfo(message);
    Diligent::Log::Instance().Draw();

    //Time of the great test
    Diligent::RigidbodyComponent* infoBody = static_cast<Diligent::RigidbodyComponent*>(info.body->getUserData());
    bodyTouch                              = infoBody;
    //message                                = "Actor name = " + infoBody->GetOwner()->GetActorName();
    messs = "Hit point : ";

  
    // Return a fraction of 1.0 to gather all hits
    return decimal(1.0);
}