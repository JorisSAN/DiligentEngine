#include "MyRaycastCallback.h"

decimal MyRaycastCallback::notifyRaycastHit(const RaycastInfo& info)
{
    // Display the world hit point coordinates
    std::string message = "Hit point : " + std::to_string(info.worldPoint.x) + " - " + std::to_string(info.worldPoint.y) + " - " + std::to_string(info.worldPoint.z);

    Diligent::Log::Instance().addInfo(message);
    Diligent::Log::Instance().Draw();

    // Return a fraction of 1.0 to gather all hits
    return decimal(1.0);
}