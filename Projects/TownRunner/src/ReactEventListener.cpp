#include "ReactEventListener.h"
#include "RigidbodyComponent.hpp"
#include "Actor.h"

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
        string message = "Actor1 name = " + infoBody1->GetOwner()->GetActorName() +
            " - Actor2 name = " + infoBody2->GetOwner()->GetActorName();

        Diligent::Log::Instance().addInfo(message);
        Diligent::Log::Instance().Draw();
    }
}
}