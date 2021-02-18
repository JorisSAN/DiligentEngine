#include "ReactEventListener.h"

namespace reactphysics3d
{
void ReactEventListener::onTrigger(const OverlapCallback::CallbackData& callbackData)
{
    // For each triggered pair
    for (uint p = 0; p < callbackData.getNbOverlappingPairs(); p++)
    {
        // Get the overlap pair
        OverlapCallback::OverlapPair overlapPair = callbackData.getOverlappingPair(p);

        // For each body of the overlap pair, we will print the name
        Diligent::RigidbodyComponent* infoBody1 = static_cast<Diligent::RigidbodyComponent*>(overlapPair.getBody1()->getUserData());
        Diligent::RigidbodyComponent* infoBody2 = static_cast<Diligent::RigidbodyComponent*>(overlapPair.getBody2()->getUserData());
        
        //Find actor1 type
        string actor1String = "";
        string actor2String = "";
        Diligent::Actor::ActorType actor1Type = Diligent::Actor::ActorType::BaseActor;
        Diligent::Actor::ActorType actor2Type = Diligent::Actor::ActorType::BaseActor;
        FindActorType(infoBody1, infoBody2, &actor1String, &actor2String, &actor1Type, &actor2Type);
        
        /*
        string message = "Actor1 name = " + actor1String +
            " - Actor2 name = " + actor2String;
        Diligent::Log::Instance().addInfo(message);
        Diligent::Log::Instance().Draw();
        */

        //Check what is the class of the overlapping body
        FindBehaviour(infoBody1, infoBody2, actor1Type, actor2Type);
    }
}


void ReactEventListener::FindActorType(Diligent::RigidbodyComponent* infoBody1, Diligent::RigidbodyComponent* infoBody2, string* actor1String, string* actor2String, Diligent::Actor::ActorType* actor1Type, Diligent::Actor::ActorType* actor2Type)
{
    switch (infoBody1->GetOwner()->GetActorType())
    {
        case Diligent::Actor::ActorType::AmbientLight:
            *actor1String = "AmbientLight";
            *actor1Type   = Diligent::Actor::ActorType::AmbientLight;
            break;

        case Diligent::Actor::ActorType::AnimPeople:
            *actor1String = "AnimPeople";
            *actor1Type   = Diligent::Actor::ActorType::AnimPeople;
            break;

        case Diligent::Actor::ActorType::BasicMesh:
            *actor1String = "BasicMesh";
            *actor1Type   = Diligent::Actor::ActorType::BasicMesh;
            break;

        case Diligent::Actor::ActorType::GLTFObject:
            *actor1String = "GLTFObject";
            *actor1Type   = Diligent::Actor::ActorType::GLTFObject;
            break;

        case Diligent::Actor::ActorType::Helmet:
            *actor1String = "Helmet";
            *actor1Type   = Diligent::Actor::ActorType::Helmet;
            break;

        case Diligent::Actor::ActorType::Plane:
            *actor1String = "Plane";
            *actor1Type   = Diligent::Actor::ActorType::Plane;
            break;

        case Diligent::Actor::ActorType::Player:
            *actor1String = "Player";
            *actor1Type   = Diligent::Actor::ActorType::Player;
            break;

        case Diligent::Actor::ActorType::PointLight:
            *actor1String = "PointLight";
            *actor1Type   = Diligent::Actor::ActorType::PointLight;
            break;

        case Diligent::Actor::ActorType::Ray:
            *actor1String = "Ray";
            *actor1Type   = Diligent::Actor::ActorType::Ray;
            break;

        case Diligent::Actor::ActorType::Sphere:
            *actor1String = "Sphere";
            *actor1Type   = Diligent::Actor::ActorType::Sphere;
            break;

        case Diligent::Actor::ActorType::Target:
            *actor1String = "Target";
            *actor1Type   = Diligent::Actor::ActorType::Target;
            break;

        default:
            *actor1String = "BaseActor";
            break;
    }

    //Find actor1 type
    switch (infoBody2->GetOwner()->GetActorType())
    {
        case Diligent::Actor::ActorType::AmbientLight:
            *actor2String = "AmbientLight";
            *actor2Type   = Diligent::Actor::ActorType::AmbientLight;
            break;

        case Diligent::Actor::ActorType::AnimPeople:
            *actor2String = "AnimPeople";
            *actor2Type   = Diligent::Actor::ActorType::AnimPeople;
            break;

        case Diligent::Actor::ActorType::BasicMesh:
            *actor2String = "BasicMesh";
            *actor2Type   = Diligent::Actor::ActorType::BasicMesh;
            break;

        case Diligent::Actor::ActorType::GLTFObject:
            *actor2String = "GLTFObject";
            *actor2Type   = Diligent::Actor::ActorType::GLTFObject;
            break;

        case Diligent::Actor::ActorType::Helmet:
            *actor2String = "Helmet";
            *actor2Type   = Diligent::Actor::ActorType::Helmet;
            break;

        case Diligent::Actor::ActorType::Plane:
            *actor2String = "Plane";
            *actor2Type   = Diligent::Actor::ActorType::Plane;
            break;

        case Diligent::Actor::ActorType::Player:
            *actor2String = "Player";
            *actor2Type   = Diligent::Actor::ActorType::Player;
            break;

        case Diligent::Actor::ActorType::PointLight:
            *actor2String = "PointLight";
            *actor2Type   = Diligent::Actor::ActorType::PointLight;
            break;

        case Diligent::Actor::ActorType::Ray:
            *actor2String = "Ray";
            *actor2Type   = Diligent::Actor::ActorType::Ray;
            break;

        case Diligent::Actor::ActorType::Sphere:
            *actor2String = "Sphere";
            *actor2Type   = Diligent::Actor::ActorType::Sphere;
            break;

        case Diligent::Actor::ActorType::Target:
            *actor2String = "Target";
            *actor2Type   = Diligent::Actor::ActorType::Target;
            break;

        default:
            *actor2String = "BaseActor";
            break;
    }
}


void ReactEventListener::FindBehaviour(Diligent::RigidbodyComponent* infoBody1, Diligent::RigidbodyComponent* infoBody2, Diligent::Actor::ActorType actor1Type, Diligent::Actor::ActorType actor2Type)
{   
    //Case if a player hit the plane
    if ((actor1Type == Diligent::Actor::ActorType::Plane && actor2Type == Diligent::Actor::ActorType::Player) || (actor1Type == Diligent::Actor::ActorType::Player && actor2Type == Diligent::Actor::ActorType::Plane))
    {
        Diligent::Player* currPlayer;
        if (actor1Type == Diligent::Actor::ActorType::Player)
        {
            currPlayer = static_cast<Diligent::Player*>(infoBody1->GetOwner());
            currPlayer->AllowJump();
        }
        else
        {
            currPlayer = static_cast<Diligent::Player*>(infoBody2->GetOwner());
            currPlayer->AllowJump();
        }
        return;
    }
}
}