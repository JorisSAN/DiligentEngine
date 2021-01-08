#pragma once
#include "Log.h"
#include <reactphysics3d/engine/EventListener.h>

using namespace reactphysics3d;

namespace Diligent
{

class ReactEventListener : public EventListener
{
    //This funciton will be called when a trigger will happend
    virtual void onTrigger(const OverlapCallback::CallbackData& callbackData) override;
};

}

