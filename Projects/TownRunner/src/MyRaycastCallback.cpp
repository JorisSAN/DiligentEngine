#include "MyRaycastCallback.h"

decimal MyRaycastCallback::notifyRaycastHit(const RaycastInfo& info)
{
    // Display the world hit point coordinates
    std::cout << "Hit point : " << info.worldPoint.x << info.worldPoint.y << info.worldPoint.z << std::endl;

    // Return a fraction of 1.0 to gather all hits
    return decimal(1.0);
}