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

class PointLight : public Actor
{
public:
    PointLight();
    PointLight(const SampleInitInfo& InitInfo, RefCntAutoPtr<IRenderPass>& RenderPass, IShaderSourceInputStreamFactory* pShaderSourceFactory);

    virtual void GetEngineInitializationAttribs(RENDER_DEVICE_TYPE DeviceType,
                                                EngineCreateInfo&  Attribs,
                                                SwapChainDesc&     SCDesc) override final;

    void Initialize(const SampleInitInfo&            InitInfo,
                    IShaderSourceInputStreamFactory* pShaderSourceFactory);

    void RenderActor(const Camera& camera, bool IsShadowPass) override;
    void UpdateActor(double CurrTime, double ElapsedTime) override;

    void setLocation(float3 location) { Location = location; }
    void setSize(float size) { Size = size; }
    void setColor(float3 color) { Color = color; }

    void CreateSRB(RefCntAutoPtr<ITexture> pColorBuffer, RefCntAutoPtr<ITexture> pDepthZBuffer);

    RefCntAutoPtr<IBuffer> getShaderConstantsCB() { return m_pShaderConstantsCB; }

private:
    void CreateLightVolumePSO(IShaderSourceInputStreamFactory* pShaderSourceFactory);
    void ApplyLighting();
    void CreateLightsBuffer();
    void InitLights();
    void ReleaseWindowResources();

    struct LightAttribs
    {
        float3 Location;
        float  Size;
        float3 Color;
    };

    // Cube resources
    RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
    RefCntAutoPtr<IBuffer>                m_pShaderConstantsCB;
    RefCntAutoPtr<ITextureView>           m_CubeTextureSRV;

    RefCntAutoPtr<IBuffer> m_pLightsBuffer;

    RefCntAutoPtr<IPipelineState>         m_pLightVolumePSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pLightVolumeSRB;

    RefCntAutoPtr<IRenderPass> m_pRenderPass;

    int  m_LightsCount      = 1;
    bool m_ShowLightVolumes = false;

    float3 Location = float3(0, 0, 0);
    float  Size     = 1;
    float3 Color    = float3(1, 1, 1);

    std::vector<LightAttribs> m_Lights;
};

} // namespace Diligent
