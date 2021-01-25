#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class Cube : public GLTFObject
{
public:
    Cube(const SampleInitInfo& InitInfo);

    void UpdateActor(double CurrTime, double ElapsedTime) override;

    
    char* getClassName() override{return "Cube";} 
};

} // namespace Diligent