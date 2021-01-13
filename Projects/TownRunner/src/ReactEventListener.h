#pragma once
#include "Log.h"
#include <reactphysics3d/engine/EventListener.h>


namespace reactphysics3d
{

class ReactEventListener : public EventListener
{
    //This funciton will be called when a trigger will happend
    virtual void onTrigger(const OverlapCallback::CallbackData& callbackData) override;
};

}

