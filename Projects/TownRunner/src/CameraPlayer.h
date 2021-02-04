#pragma once
#ifndef CAMERAPLAYER_H
#define CAMERAPLAYER_H

#include "Camera.h"

namespace Diligent
{

class CameraPlayer : public Camera
{
    public:
    virtual void Update(InputController& Controller, float ElapsedTime);
 
};
}

#endif