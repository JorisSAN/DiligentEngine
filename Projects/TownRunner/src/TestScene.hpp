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
#include "Actor.h"
#include "Camera.h"
#include "EnvMap.h"
#include "EnvMapP.h"
#include "ReactPhysic.hpp"
#include "Component.h"
#include "RigidbodyComponent.hpp"
#include "AmbientLight.h"
#include "PointLight.h"
#include "Player.h"
#include "Target.h"
#include "ReactEventListener.h"
#include "Building.h"

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

    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;

    //Needed when we will implement target actor or else
    void           CreateAdaptedActor(std::string actorClass, const SampleInitInfo& InitInfo){};


    void           CreateTargetAndLight();
    //Needed to create basic static mesh of a gltf model 
    void           CreateBasicMesh(const char* path, const SampleInitInfo& InitInfo,float3 coord);
    void           SetLastActorTransform(float3 _coord, Quaternion _quat, float _scale);


    SampleInitInfo getInitInfo() { return Init; }

    void removeActor(Actor* actor);
    void addActor(Actor* actor);
    void addRay(float3 beginPoint, float3 endPoint, float detail, float scale);

    virtual const Char* GetSampleName() const override final { return "Scene"; }

    reactphysics3d::Vector3 TestScene::GetScaleBox(const char* path);

private:
    // Use 16-bit format to make sure it works on mobile devices
    static constexpr TEXTURE_FORMAT DepthBufferFormat = TEX_FORMAT_D32_FLOAT;

    RefCntAutoPtr<IRenderPass> m_pRenderPass;

    std::unordered_map<ITextureView*, RefCntAutoPtr<IFramebuffer>> m_FramebufferCache;
    //Attributes 
    RefCntAutoPtr<IBuffer> m_CameraAttribsCB;

    BackgroundMode m_BackgroundMode = BackgroundMode::EnvironmentMap;

    MouseState m_LastMouseState;
    Player*             _player;


    std::vector<Actor*> actors;
    std::vector<Target*> targets;

    std::unique_ptr<EnvMap>       envMaps;
    std::unique_ptr<AmbientLight> ambientlight;
    std::vector<PointLight*>      lights;

    RefCntAutoPtr<ITexture> ColorBuffer;
    RefCntAutoPtr<ITexture> DepthZBuffer;
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;

    SampleInitInfo Init;

    
    
    //React physic 3d
    ReactPhysic* _reactPhysic;

    //ReactPhysics event listener (trigger)
    ReactEventListener _listener;


    //Functions
    void ActorCreation();
    RigidbodyComponent* RigidbodyComponentCreation(Actor* actor, reactphysics3d::Transform transform, BodyType type = BodyType::DYNAMIC);
    void                CollisionComponentCreation(Actor* actor, RigidbodyComponent* rb, CollisionShape* shape, reactphysics3d::Transform transform);
    void                CreateRenderPass();
    RefCntAutoPtr<IFramebuffer> CreateFramebuffer(ITextureView* pDstRenderTarget);
    IFramebuffer*               GetCurrentFramebuffer();
};

} // namespace Diligent
