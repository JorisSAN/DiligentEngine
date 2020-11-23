#pragma once
#include "Actor.h"
#include "GLTFLoader.hpp"
#include "GLTF_PBR_Renderer.hpp"
#include "ShadowMapManager.hpp"
#include "Camera.h"
#include "EnvMap.h"

namespace Diligent
{
#include "Shaders/Common/public/BasicStructures.fxh"

class GLTFObject : public Actor
{
public:
    GLTFObject();
    GLTFObject(const SampleInitInfo& InitInfo);

    void Initialize(const SampleInitInfo& InitInfo) override;

    void setObjectPath(const char* path);

    void RenderActor(const Camera& cameraP, bool IsShadowPass) override;

    void UpdateActor(double CurrTime, double ElapsedTime) override;

protected:
    const char* path;

    BackgroundMode m_BackgroundMode = BackgroundMode::PrefilteredEnvMap;


private:
    void CreatePSO() override;
    void CreateVertexBuffer() override;
    void LoadModel(const char* Path);
    void CreateShadowMap();
    void InitializeResourceBindings();
    void RenderShadowMap();

    GLTF_PBR_Renderer::RenderInfo m_RenderParams;

    struct ShadowSettings
    {
        bool           SnapCascades         = true;
        bool           StabilizeExtents     = true;
        bool           EqualizeExtents      = true;
        bool           SearchBestCascade    = true;
        bool           FilterAcrossCascades = true;
        int            Resolution           = 2048;
        float          PartitioningFactor   = 0.95f;
        TEXTURE_FORMAT Format               = TEX_FORMAT_D16_UNORM;
        int            iShadowMode          = SHADOW_MODE_PCF;

        bool Is32BitFilterableFmt = true;
    } m_ShadowSettings;

    RefCntAutoPtr<ISampler> m_pComparisonSampler;
    RefCntAutoPtr<ISampler> m_pFilterableShadowMapSampler;
    ShadowMapManager m_ShadowMapMgr;
    RefCntAutoPtr<IShaderResourceBinding>      m_ShadowSRB;
    RefCntAutoPtr<IPipelineState> m_RenderShadowPSO;

    Camera camera;

    LightAttribs m_LightAttribs;
    float4 m_LightColor     = float4(1, 1, 1, 1);
    float  m_EnvMapMipLevel = 1.f;

    static const std::pair<const char*, const char*> GLTFModels[];

    bool               m_PlayAnimation  = false;
    int                m_AnimationIndex = 0;
    std::vector<float> m_AnimationTimers;

    std::unique_ptr<GLTF_PBR_Renderer>    m_GLTFRenderer;
    std::unique_ptr<GLTF::Model>          m_Model;

    enum class BoundBoxMode : int {
            None = 0,
            Local,
            Global
    };

    BoundBoxMode m_BoundBoxMode = BoundBoxMode::None;

    MouseState m_LastMouseState;
};

} // namespace Diligent