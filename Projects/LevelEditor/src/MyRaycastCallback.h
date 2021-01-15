#pragma once
#include "ReactPhysic.hpp"
#include "Log.h"

using namespace reactphysics3d;

class MyRaycastCallback : public RaycastCallback
{
public:
    MyRaycastCallback()
    {
        messs = "is something";
    };
    //method will be called for each collider that is hit by the ray
    virtual decimal notifyRaycastHit(const RaycastInfo& info);
    std::string           messs ;
};