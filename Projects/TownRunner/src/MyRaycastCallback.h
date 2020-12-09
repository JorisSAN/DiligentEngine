#pragma once
#include "ReactPhysic.hpp"

using namespace reactphysics3d;

class MyRaycastCallback : public RaycastCallback
{
public:
    //method will be called for each collider that is hit by the ray
    virtual decimal notifyRaycastHit(const RaycastInfo& info);
};