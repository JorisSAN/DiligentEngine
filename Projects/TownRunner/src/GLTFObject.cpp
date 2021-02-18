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

#include "Shaders/Common/public/BasicStructures.fxh"
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
    _actorType = ActorType::GLTFObject;
}

GLTFObject::GLTFObject(const SampleInitInfo& InitInfo, RefCntAutoPtr<IRenderPass>& RenderPass)
{
    Initialize(InitInfo, RenderPass);
    _actorType = ActorType::GLTFObject;
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

void GLTFObject::Initialize(const SampleInitInfo& InitInfo, RefCntAutoPtr<IRenderPass>& RenderPass)
{
    SampleBase::Initialize(InitInfo);

    m_pRenderPass = RenderPass;

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
    m_GLTFRenderer.reset(new GLTF_PBR_Renderer(m_pDevice, m_pImmediateContext, RendererCI, m_pRenderPass));

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

    m_LightDirection = normalize(float3(0.5f, -0.6f, -0.2f));
}

void GLTFObject::setObjectPath(const char* pathP)
{
    path = pathP;
    LoadModel(path);
}

// Render a frame
void GLTFObject::RenderActor(const Camera& camera, bool IsShadowPass)
{
    if (state == ActorState::Active)
    {
        {
            MapHelper<LightAttribs> lightAttribs(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
            lightAttribs->f4Direction = m_LightDirection;
            lightAttribs->f4Intensity = m_LightColor * m_LightIntensity;
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
}

} // namespace Diligent
