#pragma once
#include "Actor.h"
#include "Camera.h"
#include "GLTFObject.h"

namespace Diligent
{

class Ray : public GLTFObject
{
public:
    Ray(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass);

    Ray(const SampleInitInfo& InitInfo, BackgroundMode backGroundP, RefCntAutoPtr<IRenderPass>& RenderPass, std::string name);

    void UpdateActor(double CurrTime, double ElapsedTime) override;

    /*
    void Initialize(const SampleInitInfo& InitInfo) override;

    void RenderActor(const Camera& CameraViewProj, bool IsShadowPass) override;

    private:
    void CreatePSO() override;
    */
    
};

} // namespace Diligent