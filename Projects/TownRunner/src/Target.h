#pragma once
#include "GLTFObject.h"

namespace Diligent
{

class Target : public GLTFObject
{
public:
    Target(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass);

    Target(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass, std::string name);


    void UpdateActor(double CurrTime, double ElapsedTime) override;
};

} // namespace Diligent