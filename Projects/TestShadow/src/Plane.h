#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class Plane : public GLTFObject
{
public:
    Plane(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass);

    void UpdateActor(double CurrTime, double ElapsedTime) override;
};

} // namespace Diligent