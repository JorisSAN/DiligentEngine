#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class BasicMesh : public GLTFObject
{
public:
    BasicMesh(const SampleInitInfo& InitInfo, const char* path, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass);

    void UpdateActor(double CurrTime, double ElapsedTime) override;

    std::string getPath() { return pathName; };

private:
    std::string pathName;
};

} // namespace Diligent