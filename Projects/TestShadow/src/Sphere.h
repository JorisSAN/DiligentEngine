#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class Sphere : public GLTFObject
{
public:
    Sphere(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass);

    void UpdateActor(double CurrTime, double ElapsedTime) override;
};

} // namespace Diligent