#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class AnimPeople : public GLTFObject
{
public:
    AnimPeople(const SampleInitInfo& InitInfo, BackgroundMode backGround);

    void UpdateActor(double CurrTime, double ElapsedTime) override;
};

} // namespace Diligent