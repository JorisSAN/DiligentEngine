#pragma once
#include "Log.h"
#include <reactphysics3d/engine/EventListener.h>
#include "Actor.h"
#include "RigidbodyComponent.hpp"
#include "Player.h"


namespace reactphysics3d
{

class ReactEventListener : public EventListener
{
    //This funciton will be called when a trigger will happend
    virtual void onTrigger(const OverlapCallback::CallbackData& callbackData) override;

    //Find actor type
    void FindActorType(Diligent::RigidbodyComponent* infoBody1, Diligent::RigidbodyComponent* infoBody2, string* actor1String, string* actor2String, Diligent::Actor::ActorType* actor1Type, Diligent::Actor::ActorType* actor2Type);

    //Find Behaviour
    void FindBehaviour(Diligent::RigidbodyComponent* infoBody1, Diligent::RigidbodyComponent* infoBody2, Diligent::Actor::ActorType actor1Type, Diligent::Actor::ActorType actor2Type);
};

}

