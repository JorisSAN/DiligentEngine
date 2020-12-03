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

#pragma once

#include <unordered_map>
#include <vector>

#include "Actor.h"
#include "BasicMath.hpp"

namespace Diligent
{

class Light : public Actor
{
public:
    Light();
    Light(const SampleInitInfo& InitInfo);

    virtual void GetEngineInitializationAttribs(RENDER_DEVICE_TYPE DeviceType,
                                                EngineCreateInfo&  Attribs,
                                                SwapChainDesc&     SCDesc) override final;

    void Initialize(const SampleInitInfo& InitInfo) override;

    void RenderActor(const Camera& camera, bool IsShadowPass) override;
    void UpdateActor(double CurrTime, double ElapsedTime) override;

private:
    void CreateCubePSO(IShaderSourceInputStreamFactory* pShaderSourceFactory);
    void CreateLightVolumePSO(IShaderSourceInputStreamFactory* pShaderSourceFactory);
    void CreateAmbientLightPSO(IShaderSourceInputStreamFactory* pShaderSourceFactory);
    void CreateRenderPass();
    void DrawScene();
    void ApplyLighting();
    void CreateLightsBuffer();
    void UpdateLights(float fElapsedTime);
    void InitLights();
    void ReleaseWindowResources();

    RefCntAutoPtr<IFramebuffer> CreateFramebuffer(ITextureView* pDstRenderTarget);
    IFramebuffer*               GetCurrentFramebuffer();

    // Use 16-bit format to make sure it works on mobile devices
    static constexpr TEXTURE_FORMAT DepthBufferFormat = TEX_FORMAT_D16_UNORM;

    struct LightAttribs
    {
        float3 Location;
        float  Size;
        float3 Color;
    };

    // Cube resources
    RefCntAutoPtr<IPipelineState>         m_pCubePSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pCubeSRB;
    RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
    RefCntAutoPtr<IBuffer>                m_pShaderConstantsCB;
    RefCntAutoPtr<ITextureView>           m_CubeTextureSRV;

    RefCntAutoPtr<IBuffer> m_pLightsBuffer;

    RefCntAutoPtr<IPipelineState>         m_pLightVolumePSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pLightVolumeSRB;
    RefCntAutoPtr<IPipelineState>         m_pAmbientLightPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pAmbientLightSRB;

    RefCntAutoPtr<IRenderPass> m_pRenderPass;

    float4x4 m_CameraViewProjMatrix;
    float4x4 m_CameraViewProjInvMatrix;

    int  m_LightsCount      = 100;
    bool m_ShowLightVolumes = true;
    bool m_AnimateLights    = true;

    constexpr static int GridDim = 7;

    std::unordered_map<ITextureView*, RefCntAutoPtr<IFramebuffer>> m_FramebufferCache;

    std::vector<LightAttribs> m_Lights;
    std::vector<float3>       m_LightMoveDirs;
};

} // namespace Diligent
