#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class Helmet : public GLTFObject
{
public:
    Helmet(const SampleInitInfo& InitInfo, BackgroundMode backGround);

    Helmet(const SampleInitInfo& InitInfo, BackgroundMode backGround, std::string name);


    void UpdateActor(double CurrTime, double ElapsedTime) override;
};

} // namespace Diligent