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

#include <array>

#include "AmbientLight.h"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "TexturedCube.hpp"
#include "imgui.h"
#include "ImGuiUtils.hpp"
#include "FastRand.hpp"

namespace Diligent
{

namespace
{

struct ShaderConstants
{
    float4x4 ViewProjMatrix;
    float4x4 ViewProjInvMatrix;
    float4   ViewportSize;
    int      ShowLightVolumes;
};

} // namespace

AmbientLight::AmbientLight()
{

}

AmbientLight::AmbientLight(const SampleInitInfo& InitInfo, RefCntAutoPtr<IRenderPass>& RenderPass, IShaderSourceInputStreamFactory* pShaderSourceFactory) :
    m_pRenderPass(RenderPass)
{
    Initialize(InitInfo, pShaderSourceFactory);
}

void AmbientLight::CreateAmbientLightPSO(IShaderSourceInputStreamFactory* pShaderSourceFactory)
{
    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc = PSOCreateInfo.PSODesc;

    PSODesc.Name = "Ambient light PSO";

    PSOCreateInfo.GraphicsPipeline.pRenderPass  = m_pRenderPass;
    PSOCreateInfo.GraphicsPipeline.SubpassIndex = 1; // This PSO will be used within the second subpass

    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False; // Disable depth

    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    ShaderCI.UseCombinedTextureSamplers = true;

    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "Ambient light VS";
        ShaderCI.FilePath        = "ambient_light.vsh";
        m_pDevice->CreateShader(ShaderCI, &pVS);
        VERIFY_EXPR(pVS != nullptr);
    }

    const auto IsVulkan = m_pDevice->GetDeviceCaps().IsVulkanDevice();

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.SourceLanguage  = IsVulkan ? SHADER_SOURCE_LANGUAGE_GLSL : SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "Ambient light PS";
        ShaderCI.FilePath        = IsVulkan ? "ambient_light_glsl.psh" : "ambient_light_hlsl.psh";
        m_pDevice->CreateShader(ShaderCI, &pPS);
        VERIFY_EXPR(pPS != nullptr);
    }

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    // clang-format off
    ShaderResourceVariableDesc Vars[] = 
    {
        {SHADER_TYPE_PIXEL, "g_SubpassInputColor", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, "g_SubpassInputDepthZ", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    // clang-format on
    PSODesc.ResourceLayout.Variables    = Vars;
    PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pAmbientLightPSO);
    VERIFY_EXPR(m_pAmbientLightPSO != nullptr);
}

void AmbientLight::Initialize(const SampleInitInfo&            InitInfo,
                       IShaderSourceInputStreamFactory* pShaderSourceFactory)
{
    SampleBase::Initialize(InitInfo);

    CreateAmbientLightPSO(pShaderSourceFactory);
}

void AmbientLight::ReleaseWindowResources()
{
    m_pAmbientLightSRB.Release();
}

void AmbientLight::ApplyLighting()
{
    // Set the lighting PSO
    m_pImmediateContext->SetPipelineState(m_pAmbientLightPSO);

    // Commit shader resources
    m_pImmediateContext->CommitShaderResources(m_pAmbientLightSRB, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

    {
        // Draw quad
        DrawAttribs DrawAttrs;
        DrawAttrs.NumVertices = 4;
        DrawAttrs.Flags       = DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
        m_pImmediateContext->Draw(DrawAttrs);
    }
}

void AmbientLight::CreateSRB(RefCntAutoPtr<ITexture> pColorBuffer, RefCntAutoPtr<ITexture> pDepthZBuffer)
{
    // Create SRBs that reference the framebuffer textures

    if (!m_pAmbientLightSRB)
    {
        m_pAmbientLightPSO->CreateShaderResourceBinding(&m_pAmbientLightSRB, true);
        m_pAmbientLightSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_SubpassInputColor")->Set(pColorBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
        m_pAmbientLightSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_SubpassInputDepthZ")->Set(pDepthZBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    }
}

// Render a frame
void AmbientLight::RenderActor(const Camera& camera, bool IsShadowPass)
{
    ApplyLighting();
}

void AmbientLight::UpdateActor(double CurrTime, double ElapsedTime)
{
}

} // namespace Diligent
