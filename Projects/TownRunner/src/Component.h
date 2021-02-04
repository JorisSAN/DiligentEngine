#pragma once

#include "Log.h"
#include "Actor.h"

namespace Diligent
{

class Actor;

class Component
{
public:
    enum TypeID
    {
        TComponent = 0,
        TRigidbodyComponent,
        TCollisionComponent,

        NUM_COMPONENTS_TYPES
    };

    static const char* TypeNames[NUM_COMPONENTS_TYPES];
    virtual TypeID     GetType() const = 0;

    Component(Actor* ownerP, int updateOrderP = 100);
    Component() = delete;
    virtual ~Component();
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    Actor* GetOwner() { return &owner;}

    int getUpdateOrder() const { return updateOrder; }

    virtual void update(double CurrTime, double ElapsedTime);
    virtual void onUpdateWorldTransform() {}

    Log          log;

protected:
    Actor& owner;
    int    updateOrder; // Order of the component in the actor's updateComponent method
};

} // namespace Diligent
