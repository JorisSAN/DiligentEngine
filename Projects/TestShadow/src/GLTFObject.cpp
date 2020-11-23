/*
 *  Copyright 2019-2020 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include <cmath>
#include <array>
#include "GLTFObject.h"
#include "MapHelper.hpp"
#include "BasicMath.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "CommonlyUsedStates.h"
#include "ShaderMacroHelper.hpp"
#include "FileSystem.hpp"
#include "imgui.h"
#include "imGuIZMO.h"

namespace Diligent
{
#include "Shaders/PostProcess/ToneMapping/public/ToneMappingStructures.fxh"

namespace
{

struct EnvMapRenderAttribs
{
    ToneMappingAttribs TMAttribs;

    float AverageLogLum;
    float MipLevel;
    float Unusued1;
    float Unusued2;
};

} // namespace

GLTFObject::GLTFObject()
{

}

GLTFObject::GLTFObject(const SampleInitInfo& InitInfo)
{
    Initialize(InitInfo);
}

void GLTFObject::LoadModel(const char* Path)
{
    if (m_Model)
    {
        m_GLTFRenderer->ReleaseResourceBindings(*m_Model);
        m_PlayAnimation  = false;
        m_AnimationIndex = 0;
        m_AnimationTimers.clear();
    }

    m_Model.reset(new GLTF::Model(m_pDevice, m_pImmediateContext, Path));
    m_GLTFRenderer->InitializeResourceBindings(*m_Model, m_VertexBuffer, m_VSConstants);

    // Center and scale model
    float3 ModelDim{m_Model->AABBTransform[0][0], m_Model->AABBTransform[1][1], m_Model->AABBTransform[2][2]};
    float  Scale     = (1.0f / std::max(std::max(ModelDim.x, ModelDim.y), ModelDim.z)) * 0.5f;
    auto   Translate = -float3(m_Model->AABBTransform[3][0], m_Model->AABBTransform[3][1], m_Model->AABBTransform[3][2]);
    Translate += -0.5f * ModelDim;
    float4x4 InvYAxis = float4x4::Identity();
    InvYAxis._22      = -1;
    scale             = Scale;
    position          = Translate;
    m_ContextInit     = InvYAxis;

    if (!m_Model->Animations.empty())
    {
        m_AnimationTimers.resize(m_Model->Animations.size());
        m_AnimationIndex = 0;
        m_PlayAnimation  = true;
    }
}

void GLTFObject::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_LightAttribs.ShadowAttribs.iNumCascades     = 4;
    m_LightAttribs.ShadowAttribs.fFixedDepthBias  = 0.0025f;
    m_LightAttribs.ShadowAttribs.iFixedFilterSize = 5;
    m_LightAttribs.ShadowAttribs.fFilterWorldSize = 0.1f;

    m_LightAttribs.f4Direction    = float3(-0.522699475f, -0.481321275f, -0.703671455f);
    m_LightAttribs.f4Intensity    = float4(1, 0.8f, 0.5f, 1);
    m_LightAttribs.f4AmbientLight = float4(0.125f, 0.125f, 0.125f, 1);

    RefCntAutoPtr<ITexture> EnvironmentMap;
    CreateTextureFromFile("textures/papermill.ktx", TextureLoadInfo{"Environment map"}, m_pDevice, &EnvironmentMap);
    m_TextureSRV = EnvironmentMap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    auto BackBufferFmt  = m_pSwapChain->GetDesc().ColorBufferFormat;
    auto DepthBufferFmt = m_pSwapChain->GetDesc().DepthBufferFormat;

    GLTF_PBR_Renderer::CreateInfo RendererCI;
    RendererCI.RTVFmt         = BackBufferFmt;
    RendererCI.DSVFmt         = DepthBufferFmt;
    RendererCI.AllowDebugView = true;
    RendererCI.UseIBL         = true;
    RendererCI.FrontCCW       = true;
    m_GLTFRenderer.reset(new GLTF_PBR_Renderer(m_pDevice, m_pImmediateContext, RendererCI));

    CreateUniformBuffer(m_pDevice, sizeof(CameraAttribs), "Camera attribs buffer", &m_VertexBuffer);
    CreateUniformBuffer(m_pDevice, sizeof(LightAttribs), "Light attribs buffer", &m_VSConstants);
    CreateUniformBuffer(m_pDevice, sizeof(EnvMapRenderAttribs), "Env map render attribs buffer", &m_IndexBuffer);
    // clang-format off
    StateTransitionDesc Barriers [] =
    {
        {m_VertexBuffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
        {m_VSConstants,  RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
        {m_IndexBuffer,  RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
        {EnvironmentMap, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, true}
    };
    // clang-format on
    m_pImmediateContext->TransitionResourceStates(_countof(Barriers), Barriers);

    m_GLTFRenderer->PrecomputeCubemaps(m_pDevice, m_pImmediateContext, m_TextureSRV);

    CreatePSO();

    CreateShadowMap();
}

void GLTFObject::setObjectPath(const char* pathP)
{
    path = pathP;
    LoadModel(path);
}

void GLTFObject::CreatePSO()
{
    ShaderCreateInfo                               ShaderCI;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_pEngineFactory->CreateDefaultShaderSourceStreamFactory("shaders", &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    ShaderCI.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;
    ShaderCI.UseCombinedTextureSamplers = true;

    ShaderMacroHelper Macros;
    // clang-format off
    Macros.AddShaderMacro( "SHADOW_MODE",            m_ShadowSettings.iShadowMode);
    Macros.AddShaderMacro( "SHADOW_FILTER_SIZE",     m_LightAttribs.ShadowAttribs.iFixedFilterSize);
    Macros.AddShaderMacro( "FILTER_ACROSS_CASCADES", m_ShadowSettings.FilterAcrossCascades);
    Macros.AddShaderMacro( "BEST_CASCADE_SEARCH",    m_ShadowSettings.SearchBestCascade );
    // clang-format on
    Macros.AddShaderMacro("TONE_MAPPING_MODE", "TONE_MAPPING_MODE_UNCHARTED2");
    ShaderCI.Macros = Macros;

    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.Desc.Name       = "Environment map VS";
    ShaderCI.EntryPoint      = "main";
    ShaderCI.FilePath        = "env_map.vsh";
    RefCntAutoPtr<IShader> pVS;
    m_pDevice->CreateShader(ShaderCI, &pVS);

    ShaderCI.Desc.Name       = "Environment map PS";
    ShaderCI.EntryPoint      = "main";
    ShaderCI.FilePath        = "env_map.psh";
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    RefCntAutoPtr<IShader> pPS;
    m_pDevice->CreateShader(ShaderCI, &pPS);

    Macros.AddShaderMacro("SHADOW_PASS", true);
    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.Desc.Name       = "Environment map VS";
    ShaderCI.EntryPoint      = "main";
    ShaderCI.FilePath        = "env_map.vsh";
    ShaderCI.Macros          = Macros;
    RefCntAutoPtr<IShader> pShadowVS;
    m_pDevice->CreateShader(ShaderCI, &pShadowVS);

    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc          = PSOCreateInfo.PSODesc;
    GraphicsPipelineDesc&           GraphicsPipeline = PSOCreateInfo.GraphicsPipeline;

    PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

    // clang-format off
    ImmutableSamplerDesc ImmutableSamplers[] =
    {
        {SHADER_TYPE_PIXEL, "EnvMap", Sam_LinearClamp},
        {SHADER_TYPE_PIXEL, "g_tex2DDiffuse", Sam_Aniso4xWrap}
    };
    // clang-format on
    PSODesc.ResourceLayout.ImmutableSamplers    = ImmutableSamplers;
    PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImmutableSamplers);

    // clang-format off
    ShaderResourceVariableDesc Vars[] = 
    {
        {SHADER_TYPE_PIXEL, "cbCameraAttribs",       SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, "cbEnvMapRenderAttribs", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, m_ShadowSettings.iShadowMode == SHADOW_MODE_PCF ? "g_tex2DShadowMap" : "g_tex2DFilterableShadowMap", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    // clang-format on
    PSODesc.ResourceLayout.Variables    = Vars;
    PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    PSODesc.Name      = "EnvMap PSO";
    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    GraphicsPipeline.RTVFormats[0]              = m_pSwapChain->GetDesc().ColorBufferFormat;
    GraphicsPipeline.NumRenderTargets           = 1;
    GraphicsPipeline.DSVFormat                  = m_pSwapChain->GetDesc().DepthBufferFormat;
    GraphicsPipeline.PrimitiveTopology          = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

    m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);
    m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbCameraAttribs")->Set(m_VertexBuffer);
    m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbEnvMapRenderAttribs")->Set(m_IndexBuffer);
    CreateVertexBuffer();
}

void GLTFObject::InitializeResourceBindings()
{
    {
        RefCntAutoPtr<IShaderResourceBinding> pSRB;
        m_pPSO->CreateShaderResourceBinding(&pSRB, true);
        pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex2DDiffuse")->Set(m_TextureSRV);
        if (m_ShadowSettings.iShadowMode == SHADOW_MODE_PCF)
        {
            pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex2DShadowMap")->Set(m_ShadowMapMgr.GetSRV());
        }
        else
        {
            pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex2DFilterableShadowMap")->Set(m_ShadowMapMgr.GetFilterableSRV());
        }
        m_SRB = std::move(pSRB);
    }

    {
        RefCntAutoPtr<IShaderResourceBinding> pShadowSRB;
        m_RenderShadowPSO->CreateShaderResourceBinding(&pShadowSRB, true);
        m_ShadowSRB = std::move(pShadowSRB);
    }
}

void GLTFObject::CreateShadowMap()
{
    if (m_ShadowSettings.Resolution >= 2048)
        m_LightAttribs.ShadowAttribs.fFixedDepthBias = 0.0025f;
    else if (m_ShadowSettings.Resolution >= 1024)
        m_LightAttribs.ShadowAttribs.fFixedDepthBias = 0.005f;
    else
        m_LightAttribs.ShadowAttribs.fFixedDepthBias = 0.0075f;

    ShadowMapManager::InitInfo SMMgrInitInfo;
    SMMgrInitInfo.Format               = m_ShadowSettings.Format;
    SMMgrInitInfo.Resolution           = m_ShadowSettings.Resolution;
    SMMgrInitInfo.NumCascades          = static_cast<Uint32>(m_LightAttribs.ShadowAttribs.iNumCascades);
    SMMgrInitInfo.ShadowMode           = m_ShadowSettings.iShadowMode;
    SMMgrInitInfo.Is32BitFilterableFmt = m_ShadowSettings.Is32BitFilterableFmt;

    if (!m_pComparisonSampler)
    {
        SamplerDesc ComparsionSampler;
        ComparsionSampler.ComparisonFunc = COMPARISON_FUNC_LESS;
        // Note: anisotropic filtering requires SampleGrad to fix artifacts at
        // cascade boundaries
        ComparsionSampler.MinFilter = FILTER_TYPE_COMPARISON_LINEAR;
        ComparsionSampler.MagFilter = FILTER_TYPE_COMPARISON_LINEAR;
        ComparsionSampler.MipFilter = FILTER_TYPE_COMPARISON_LINEAR;
        m_pDevice->CreateSampler(ComparsionSampler, &m_pComparisonSampler);
    }
    SMMgrInitInfo.pComparisonSampler = m_pComparisonSampler;

    if (!m_pFilterableShadowMapSampler)
    {
        SamplerDesc SamplerDesc;
        SamplerDesc.MinFilter     = FILTER_TYPE_ANISOTROPIC;
        SamplerDesc.MagFilter     = FILTER_TYPE_ANISOTROPIC;
        SamplerDesc.MipFilter     = FILTER_TYPE_ANISOTROPIC;
        SamplerDesc.MaxAnisotropy = m_LightAttribs.ShadowAttribs.iMaxAnisotropy;
        m_pDevice->CreateSampler(SamplerDesc, &m_pFilterableShadowMapSampler);
    }
    SMMgrInitInfo.pFilterableShadowMapSampler = m_pFilterableShadowMapSampler;

    m_ShadowMapMgr.Initialize(m_pDevice, SMMgrInitInfo);

    InitializeResourceBindings();
}

void GLTFObject::CreateVertexBuffer()
{
    if (m_BackgroundMode != BackgroundMode::None)
    {
        m_SRB.Release();
        m_pPSO->CreateShaderResourceBinding(&m_SRB, true);
        ITextureView* pEnvMapSRV = nullptr;
        switch (m_BackgroundMode)
        {
            case BackgroundMode::EnvironmentMap:
                pEnvMapSRV = m_TextureSRV;
                break;

            case BackgroundMode::Irradiance:
                pEnvMapSRV = m_GLTFRenderer->GetIrradianceCubeSRV();
                break;

            case BackgroundMode::PrefilteredEnvMap:
                pEnvMapSRV = m_GLTFRenderer->GetPrefilteredEnvMapSRV();
                break;

            default:
                UNEXPECTED("Unexpected background mode");
        }
        m_SRB->GetVariableByName(SHADER_TYPE_PIXEL, "EnvMap")->Set(pEnvMapSRV);
    }
}

void GLTFObject::RenderShadowMap()
{
    auto iNumShadowCascades = m_LightAttribs.ShadowAttribs.iNumCascades;
    for (int iCascade = 0; iCascade < iNumShadowCascades; ++iCascade)
    {
        const auto CascadeProjMatr = m_ShadowMapMgr.GetCascadeTranform(iCascade).Proj;

        auto WorldToLightViewSpaceMatr = m_LightAttribs.ShadowAttribs.mWorldToLightViewT.Transpose();
        auto WorldToLightProjSpaceMatr = WorldToLightViewSpaceMatr * CascadeProjMatr;

        CameraAttribs ShadowCameraAttribs = {};

        ShadowCameraAttribs.mViewT     = m_LightAttribs.ShadowAttribs.mWorldToLightViewT;
        ShadowCameraAttribs.mProjT     = CascadeProjMatr.Transpose();
        ShadowCameraAttribs.mViewProjT = WorldToLightProjSpaceMatr.Transpose();

        ShadowCameraAttribs.f4ViewportSize.x = static_cast<float>(m_ShadowSettings.Resolution);
        ShadowCameraAttribs.f4ViewportSize.y = static_cast<float>(m_ShadowSettings.Resolution);
        ShadowCameraAttribs.f4ViewportSize.z = 1.f / ShadowCameraAttribs.f4ViewportSize.x;
        ShadowCameraAttribs.f4ViewportSize.w = 1.f / ShadowCameraAttribs.f4ViewportSize.y;

        {
            MapHelper<CameraAttribs> CameraData(m_pImmediateContext, m_VertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
            *CameraData = ShadowCameraAttribs;
        }

        auto* pCascadeDSV = m_ShadowMapMgr.GetCascadeDSV(iCascade);
        m_pImmediateContext->SetRenderTargets(0, nullptr, pCascadeDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pCascadeDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    if (m_ShadowSettings.iShadowMode > SHADOW_MODE_PCF)
        m_ShadowMapMgr.ConvertToFilterable(m_pImmediateContext, m_LightAttribs.ShadowAttribs);
}

// Render a frame
void GLTFObject::RenderActor(const Camera& cameraP, bool IsShadowPass)
{
    camera = cameraP;

    RenderShadowMap();

    {
        MapHelper<LightAttribs> lightAttribs(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
        lightAttribs->f4Direction = m_LightAttribs.f4Direction;
        lightAttribs->f4Intensity = m_LightAttribs.f4Intensity;
    }

    // Get pretransform matrix that rotates the scene according the surface orientation
    auto SrfPreTransform = GetSurfacePretransformMatrix(float3{0, 0, 1});

    const auto  CameraView     = camera.m_ViewMatrix * SrfPreTransform;
    const auto& CameraWorld    = camera.GetWorldMatrix();
    float3      CameraWorldPos = float3::MakeVector(CameraWorld[3]);
    const auto& Proj           = GetAdjustedProjectionMatrix(PI_F / 4.0f, 0.1f, 100.f);

    auto CameraViewProj = CameraView * Proj;

    m_RenderParams.ModelTransform = m_WorldMatrix;

    {
        MapHelper<CameraAttribs> CamAttribs(m_pImmediateContext, m_VertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        CamAttribs->mProjT        = Proj.Transpose();
        CamAttribs->mViewProjT    = CameraViewProj.Transpose();
        CamAttribs->mViewProjInvT = CameraViewProj.Inverse().Transpose();
        CamAttribs->f4Position    = float4(CameraWorldPos, 1);
    }

    m_GLTFRenderer->Render(m_pImmediateContext, *m_Model, m_RenderParams);

    if (m_BackgroundMode != BackgroundMode::None)
    {
        {
            MapHelper<EnvMapRenderAttribs> EnvMapAttribs(m_pImmediateContext, m_IndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
            EnvMapAttribs->TMAttribs.iToneMappingMode     = TONE_MAPPING_MODE_UNCHARTED2;
            EnvMapAttribs->TMAttribs.bAutoExposure        = 0;
            EnvMapAttribs->TMAttribs.fMiddleGray          = m_RenderParams.MiddleGray;
            EnvMapAttribs->TMAttribs.bLightAdaptation     = 0;
            EnvMapAttribs->TMAttribs.fWhitePoint          = m_RenderParams.WhitePoint;
            EnvMapAttribs->TMAttribs.fLuminanceSaturation = 1.0;
            EnvMapAttribs->AverageLogLum                  = m_RenderParams.AverageLogLum;
            EnvMapAttribs->MipLevel                       = m_EnvMapMipLevel;
        }
        m_pImmediateContext->SetPipelineState(m_pPSO);
        m_pImmediateContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        DrawAttribs drawAttribs(3, DRAW_FLAG_VERIFY_ALL);
        m_pImmediateContext->Draw(drawAttribs);
    }
}

void GLTFObject::UpdateActor(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    if (!m_Model->Animations.empty() && m_PlayAnimation)
    {
        float& AnimationTimer = m_AnimationTimers[m_AnimationIndex];
        AnimationTimer += static_cast<float>(ElapsedTime);
        AnimationTimer = std::fmod(AnimationTimer, m_Model->Animations[m_AnimationIndex].End);
        m_Model->UpdateAnimation(m_AnimationIndex, AnimationTimer);
    }

    ShadowMapManager::DistributeCascadeInfo DistrInfo;
    DistrInfo.pCameraView   = &camera.GetViewMatrix();
    DistrInfo.pCameraProj   = &camera.GetProjMatrix();
    float3 f3LightDirection = float3(m_LightAttribs.f4Direction.x, m_LightAttribs.f4Direction.y, m_LightAttribs.f4Direction.z);
    DistrInfo.pLightDir     = &f3LightDirection;

    DistrInfo.fPartitioningFactor = m_ShadowSettings.PartitioningFactor;
    DistrInfo.SnapCascades        = m_ShadowSettings.SnapCascades;
    DistrInfo.EqualizeExtents     = m_ShadowSettings.EqualizeExtents;
    DistrInfo.StabilizeExtents    = m_ShadowSettings.StabilizeExtents;

    m_ShadowMapMgr.DistributeCascades(DistrInfo, m_LightAttribs.ShadowAttribs);
}

} // namespace Diligent
