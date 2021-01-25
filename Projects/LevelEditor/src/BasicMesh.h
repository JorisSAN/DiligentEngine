#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class BasicMesh : public GLTFObject
{
public:
    BasicMesh(const SampleInitInfo& InitInfo,const char* path);

    void UpdateActor(double CurrTime, double ElapsedTime) override;

    char* getClassName() override { return "BasicMesh"; }
    std::string getPath() { return pathName; };

private:
    std::string pathName;
};

} // namespace Diligent