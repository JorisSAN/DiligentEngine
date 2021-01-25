#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class Plane : public GLTFObject
{
public:
    Plane(const SampleInitInfo& InitInfo);

   // Plane(const SampleInitInfo& InitInfo, std::string name);

    void UpdateActor(double CurrTime, double ElapsedTime) override;

    char* getClassName() { return "Plane"; }

};

} // namespace Diligent