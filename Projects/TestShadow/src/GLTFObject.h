#pragma once
#include "Actor.h"
#include "GLTFLoader.hpp"
#include "GLTF_PBR_Renderer.hpp"
#include "Camera.h"
#include "EnvMap.h"

namespace Diligent
{

class GLTFObject : public Actor
{
public:
    GLTFObject();
    GLTFObject(const SampleInitInfo& InitInfo, RefCntAutoPtr<IRenderPass>& RenderPass);

    ~GLTFObject();

    void Initialize(const SampleInitInfo& InitInfo, RefCntAutoPtr<IRenderPass>& RenderPass);

    void setObjectPath(const char* path);

    void RenderActor(const Camera& camera, bool IsShadowPass) override;

    void UpdateActor(double CurrTime, double ElapsedTime) override;

protected:
    const char* path;

    BackgroundMode             m_BackgroundMode = BackgroundMode::EnvironmentMap;
    RefCntAutoPtr<IRenderPass> m_pRenderPass;

private:
    void LoadModel(const char* Path);

    GLTF_PBR_Renderer::RenderInfo m_RenderParams;

    float3 m_LightDirection;
    float4 m_LightColor     = float4(1, 1, 1, 1);
    float  m_LightIntensity = 3.f;

    bool               m_PlayAnimation  = false;
    int                m_AnimationIndex = 0;
    std::vector<float> m_AnimationTimers;

    std::unique_ptr<GLTF_PBR_Renderer>    m_GLTFRenderer;
    std::unique_ptr<GLTF::Model>          m_Model;

    MouseState m_LastMouseState;
};

} // namespace Diligent