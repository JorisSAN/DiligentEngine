#pragma once
#include "Actor.h"
#include "GLTFLoader.hpp"
#include "GLTF_PBR_Renderer.hpp"
#include "Camera.h"
#include "EnvMap.h"

namespace Diligent
{

class EnvMap : public Actor
{
public:
    EnvMap();
    EnvMap(const SampleInitInfo& InitInfo, BackgroundMode BackgroundMode);

    void Initialize(const SampleInitInfo& InitInfo) override;

    void RenderActor(const Camera& camera, bool IsShadowPass) override;

protected:
    const char* path;

    BackgroundMode m_BackgroundMode = BackgroundMode::EnvironmentMap;


private:
    void CreatePSO() override;
    void CreateVertexBuffer() override;

    GLTF_PBR_Renderer::RenderInfo m_RenderParams;

    float3 m_LightDirection;
    float4 m_LightColor     = float4(1, 1, 1, 1);
    float  m_LightIntensity = 3.f;
    float  m_EnvMapMipLevel = 1.f;
    int    m_SelectedModel  = 3;

    std::unique_ptr<GLTF_PBR_Renderer>    m_GLTFRenderer;
};

} // namespace Diligent