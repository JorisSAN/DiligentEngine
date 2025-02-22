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
#include <vector>

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "Camera.h"
#include "Actor.h"
#include "Log.h"
#include "ReactPhysic.hpp"
#include "RigidbodyComponent.hpp"
#include "MyRaycastCallback.h"

namespace Diligent
{

class TestScene final : public SampleBase
{
public:
    static TestScene& instance()
    {
        static TestScene inst;
        return inst;
    }
    virtual void GetEngineInitializationAttribs(RENDER_DEVICE_TYPE DeviceType, EngineCreateInfo& EngineCI, SwapChainDesc& SCDesc) override final;
    SampleInitInfo getInitInfo() { return Init; }

    virtual void Initialize(const SampleInitInfo& InitInfo) override final;
    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    void         CreateShadowMapVisPSO();
    virtual const Char* GetSampleName() const override final { return "TestScene"; }


    ////// Level editor function
    void ReadFile(std::string fileName, const SampleInitInfo& InitInfo);
    void         CreateAdaptedActor(std::string actorClass, const SampleInitInfo& InitInfo);
    void         CreateBasicMesh(const char* path, const SampleInitInfo& InitInfo);
    void         SaveLevel(std::string fileName);
    void                UpdateUI(bool showMidUI);
    float3                Unproject(float windowsX, float windowsY,  const float4x4& modelView,const float4x4& projection);
    int                 ReturnIndexOfActor(Actor *actor);

    void removeActor(Actor* actor);

    RefCntAutoPtr<IShaderResourceBinding> m_ShadowMapVisSRB;
    RefCntAutoPtr<IPipelineState>         m_pShadowMapVisPSO;
    float4x4       m_CubeWorldMatrix;
    float4x4       m_CameraViewProjMatrix;
    float4x4       m_WorldToShadowMapUVDepthMatr;
    float3         m_LightDirection  = normalize(float3(-0.49f, -0.60f, 0.64f));
    Uint32         m_ShadowMapSize   = 512;
    TEXTURE_FORMAT m_ShadowMapFormat = TEX_FORMAT_D16_UNORM;

    // LEvel Editor var
    int                 indexActors       = -1;
    std::vector<Actor*> actors;
    char               nameSelected[32];
    char                meshSelected[64];
    char               levelName[32];
    std::vector<float3> actorsPos;
    std::vector<Quaternion> actorsRot;
    std::vector<float> actorsSca;
    MouseState              m_LastMouseStateUI;
    MouseState              m_LastMouseState;
    Camera                  m_Camera;
    Log log;
    SampleInitInfo Init;

    ReactPhysic* _reactPhysic;
    RigidbodyComponent* RigidbodyComponentCreation(Actor* actor, reactphysics3d::Transform transform, BodyType type = BodyType::DYNAMIC);
    void CollisionComponentCreation(Actor* actor, RigidbodyComponent* rb, CollisionShape* shape, reactphysics3d::Transform transform);
};

} // namespace Diligent
