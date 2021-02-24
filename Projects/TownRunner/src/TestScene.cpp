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

#include <vector>

#include <stdio.h>

#include "TestScene.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "LevelLoader.h"
#include "TextureUtilities.h"
#include "Sphere.h"
#include "Helmet.h"
#include "BasicMesh.h"
#include "AnimPeople.h"
#include "InputController.hpp"
#include "Raycast.h"
#include "Actor.h"
#include "Plane.h"
#include "Ray.h"
#include "CollisionComponent.hpp"

namespace Diligent
{

//reactphysics3d::Vector3 GetScaleBox(const char* path);
    SampleBase* CreateSample()
{
    return new TestScene();
}

void TestScene::GetEngineInitializationAttribs(RENDER_DEVICE_TYPE DeviceType, EngineCreateInfo& EngineCI, SwapChainDesc& SCDesc)
{
    SampleBase::GetEngineInitializationAttribs(DeviceType, EngineCI, SCDesc);

    EngineCI.Features.DepthClamp = DEVICE_FEATURE_STATE_OPTIONAL;
    // We do not need the depth buffer from the swap chain in this sample
    SCDesc.DepthBufferFormat = TEX_FORMAT_UNKNOWN;
}

void TestScene::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    //Initialize react physic 3d
    _reactPhysic = new ReactPhysic();
    //Initialize the event listener (trigger)
    _reactPhysic->GetPhysicWorld()->setEventListener(&_listener);

    Init = InitInfo;
    CreateRenderPass();

    //Player
    _player = new Player(Init, m_BackgroundMode, m_pRenderPass, "Player");
    _player->Initialize(float3(0, 1, 0), Quaternion(0, 0, 0, 1), _reactPhysic, float3(0, 0.5f, 0), 0.5f, 1.8f, 0.005f, 10.f, 40000);

    m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);

    envMaps.reset(new EnvMap(Init, m_BackgroundMode, m_pRenderPass));

    ambientlight.reset(new AmbientLight(Init, m_pRenderPass, pShaderSourceFactory));

    //#########################
    // Line Trace
    float3 p(0, 0, 0);
    float3 p2(1, 1, 1);

    addRay(p, p2, 10, 0.5);
    //#########################

    //ReadFile coming from levelLoader
    ReadFile("BlockoutRemake.txt", InitInfo, this);
    ActorCreation();
    
    CreateTargetAndLight();
}

void TestScene::CreateRenderPass()
{
    // Attachment 0 - Color buffer
    // Attachment 1 - Depth Z
    // Attachment 2 - Depth buffer
    // Attachment 3 - Final color buffer
    constexpr Uint32 NumAttachments = 4;

    // Prepare render pass attachment descriptions
    RenderPassAttachmentDesc Attachments[NumAttachments];
    Attachments[0].Format       = TEX_FORMAT_RGBA8_UNORM;
    Attachments[0].InitialState = RESOURCE_STATE_RENDER_TARGET;
    Attachments[0].FinalState   = RESOURCE_STATE_INPUT_ATTACHMENT;
    Attachments[0].LoadOp       = ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[0].StoreOp      = ATTACHMENT_STORE_OP_DISCARD; // We will not need the result after the end of the render pass

    Attachments[1].Format       = TEX_FORMAT_R32_FLOAT;
    Attachments[1].InitialState = RESOURCE_STATE_RENDER_TARGET;
    Attachments[1].FinalState   = RESOURCE_STATE_INPUT_ATTACHMENT;
    Attachments[1].LoadOp       = ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[1].StoreOp      = ATTACHMENT_STORE_OP_DISCARD; // We will not need the result after the end of the render pass

    Attachments[2].Format       = DepthBufferFormat;
    Attachments[2].InitialState = RESOURCE_STATE_DEPTH_WRITE;
    Attachments[2].FinalState   = RESOURCE_STATE_DEPTH_WRITE;
    Attachments[2].LoadOp       = ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[2].StoreOp      = ATTACHMENT_STORE_OP_DISCARD; // We will not need the result after the end of the render pass

    Attachments[3].Format       = m_pSwapChain->GetDesc().ColorBufferFormat;
    Attachments[3].InitialState = RESOURCE_STATE_RENDER_TARGET;
    Attachments[3].FinalState   = RESOURCE_STATE_RENDER_TARGET;
    Attachments[3].LoadOp       = ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[3].StoreOp      = ATTACHMENT_STORE_OP_STORE;

    // Subpass 1 - Render G-buffer
    // Subpass 2 - Lighting
    constexpr Uint32 NumSubpasses = 2;

    // Prepar subpass descriptions
    SubpassDesc Subpasses[NumSubpasses];

    // clang-format off
    // Subpass 0 attachments - 2 render targets and depth buffer
    AttachmentReference RTAttachmentRefs0[] =
    {
        {0, RESOURCE_STATE_RENDER_TARGET},
        {1, RESOURCE_STATE_RENDER_TARGET}
    };

    AttachmentReference DepthAttachmentRef0 = {2, RESOURCE_STATE_DEPTH_WRITE};

    // Subpass 1 attachments - 1 render target, depth buffer, 2 input attachments
    AttachmentReference RTAttachmentRefs1[] =
    {
        {3, RESOURCE_STATE_RENDER_TARGET}
    };

    AttachmentReference DepthAttachmentRef1 = {2, RESOURCE_STATE_DEPTH_WRITE};

    AttachmentReference InputAttachmentRefs1[] =
    {
        {0, RESOURCE_STATE_INPUT_ATTACHMENT},
        {1, RESOURCE_STATE_INPUT_ATTACHMENT}
    };
    // clang-format on

    Subpasses[0].RenderTargetAttachmentCount = _countof(RTAttachmentRefs0);
    Subpasses[0].pRenderTargetAttachments    = RTAttachmentRefs0;
    Subpasses[0].pDepthStencilAttachment     = &DepthAttachmentRef0;

    Subpasses[1].RenderTargetAttachmentCount = _countof(RTAttachmentRefs1);
    Subpasses[1].pRenderTargetAttachments    = RTAttachmentRefs1;
    Subpasses[1].pDepthStencilAttachment     = &DepthAttachmentRef1;
    Subpasses[1].InputAttachmentCount        = _countof(InputAttachmentRefs1);
    Subpasses[1].pInputAttachments           = InputAttachmentRefs1;

    // We need to define dependency between subpasses 0 and 1 to ensure that
    // all writes are complete before we use the attachments for input in subpass 1.
    SubpassDependencyDesc Dependencies[1];
    Dependencies[0].SrcSubpass    = 0;
    Dependencies[0].DstSubpass    = 1;
    Dependencies[0].SrcStageMask  = PIPELINE_STAGE_FLAG_RENDER_TARGET;
    Dependencies[0].DstStageMask  = PIPELINE_STAGE_FLAG_PIXEL_SHADER;
    Dependencies[0].SrcAccessMask = ACCESS_FLAG_RENDER_TARGET_WRITE;
    Dependencies[0].DstAccessMask = ACCESS_FLAG_SHADER_READ;

    RenderPassDesc RPDesc;
    RPDesc.Name            = "Deferred shading render pass desc";
    RPDesc.AttachmentCount = _countof(Attachments);
    RPDesc.pAttachments    = Attachments;
    RPDesc.SubpassCount    = _countof(Subpasses);
    RPDesc.pSubpasses      = Subpasses;
    RPDesc.DependencyCount = _countof(Dependencies);
    RPDesc.pDependencies   = Dependencies;

    m_pDevice->CreateRenderPass(RPDesc, &m_pRenderPass);
    VERIFY_EXPR(m_pRenderPass != nullptr);
}

RefCntAutoPtr<IFramebuffer> TestScene::CreateFramebuffer(ITextureView* pDstRenderTarget)
{
    const auto& RPDesc = m_pRenderPass->GetDesc();
    const auto& SCDesc = m_pSwapChain->GetDesc();
    // Create window-size offscreen render target
    TextureDesc TexDesc;
    TexDesc.Name      = "Color G-buffer";
    TexDesc.Type      = RESOURCE_DIM_TEX_2D;
    TexDesc.BindFlags = BIND_RENDER_TARGET | BIND_INPUT_ATTACHMENT;
    TexDesc.Format    = RPDesc.pAttachments[0].Format;
    TexDesc.Width     = SCDesc.Width;
    TexDesc.Height    = SCDesc.Height;
    TexDesc.MipLevels = 1;

    // Define optimal clear value
    TexDesc.ClearValue.Format   = TexDesc.Format;
    TexDesc.ClearValue.Color[0] = 0.f;
    TexDesc.ClearValue.Color[1] = 0.f;
    TexDesc.ClearValue.Color[2] = 0.f;
    TexDesc.ClearValue.Color[3] = 0.f;
    RefCntAutoPtr<ITexture> pColorBuffer;
    m_pDevice->CreateTexture(TexDesc, nullptr, &pColorBuffer);

    // OpenGL does not allow combining swap chain render target with any
    // other render target, so we have to create an auxiliary texture.
    RefCntAutoPtr<ITexture> pOpenGLOffsreenColorBuffer;
    if (pDstRenderTarget == nullptr)
    {
        TexDesc.Name   = "OpenGL Offscreen Render Target";
        TexDesc.Format = SCDesc.ColorBufferFormat;
        m_pDevice->CreateTexture(TexDesc, nullptr, &pOpenGLOffsreenColorBuffer);
        pDstRenderTarget = pOpenGLOffsreenColorBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
    }


    TexDesc.Name   = "Depth Z G-buffer";
    TexDesc.Format = RPDesc.pAttachments[1].Format;

    TexDesc.ClearValue.Format   = TexDesc.Format;
    TexDesc.ClearValue.Color[0] = 1.f;
    TexDesc.ClearValue.Color[1] = 1.f;
    TexDesc.ClearValue.Color[2] = 1.f;
    TexDesc.ClearValue.Color[3] = 1.f;
    RefCntAutoPtr<ITexture> pDepthZBuffer;
    m_pDevice->CreateTexture(TexDesc, nullptr, &pDepthZBuffer);


    TexDesc.Name      = "Depth buffer";
    TexDesc.Format    = RPDesc.pAttachments[2].Format;
    TexDesc.BindFlags = BIND_DEPTH_STENCIL;

    TexDesc.ClearValue.Format               = TexDesc.Format;
    TexDesc.ClearValue.DepthStencil.Depth   = 1.f;
    TexDesc.ClearValue.DepthStencil.Stencil = 0;
    RefCntAutoPtr<ITexture> pDepthBuffer;
    m_pDevice->CreateTexture(TexDesc, nullptr, &pDepthBuffer);


    ITextureView* pAttachments[] = //
        {
            pColorBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),
            pDepthZBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),
            pDepthBuffer->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL),
            pDstRenderTarget //
        };

    FramebufferDesc FBDesc;
    FBDesc.Name            = "G-buffer framebuffer";
    FBDesc.pRenderPass     = m_pRenderPass;
    FBDesc.AttachmentCount = _countof(pAttachments);
    FBDesc.ppAttachments   = pAttachments;

    RefCntAutoPtr<IFramebuffer> pFramebuffer;
    m_pDevice->CreateFramebuffer(FBDesc, &pFramebuffer);
    VERIFY_EXPR(pFramebuffer != nullptr);

    ColorBuffer  = pColorBuffer;
    DepthZBuffer = pDepthZBuffer;

    return pFramebuffer;
}

IFramebuffer* TestScene::GetCurrentFramebuffer()
{
    auto* pCurrentBackBufferRTV = m_pDevice->GetDeviceCaps().IsGLDevice() ?
        nullptr :
        m_pSwapChain->GetCurrentBackBufferRTV();

    auto fb_it = m_FramebufferCache.find(pCurrentBackBufferRTV);
    if (fb_it != m_FramebufferCache.end())
    {
        return fb_it->second;
    }
    else
    {
        auto it = m_FramebufferCache.emplace(pCurrentBackBufferRTV, CreateFramebuffer(pCurrentBackBufferRTV));
        VERIFY_EXPR(it.second);
        return it.first->second;
    }
}

void TestScene::CreateBasicMesh(const char* path, const SampleInitInfo& InitInfo,float3 coord)
{
    //BasicMesh* mesh = new BasicMesh(Init, path, m_BackgroundMode, m_pRenderPass);
    Building* building = new Building(Init, m_BackgroundMode, m_pRenderPass, "Building", path);
    float3     vec(coord);
    //need to correct with correct collision probably

    reactphysics3d::Transform cubeTransform(reactphysics3d::Vector3(vec.x, vec.y, vec.z), reactphysics3d::Quaternion::identity());
    reactphysics3d::Transform nullTransform(reactphysics3d::Vector3(0, 0, 0), reactphysics3d::Quaternion::identity());

    //rigid body
    RigidbodyComponent* rbCube = RigidbodyComponentCreation(building, cubeTransform, BodyType::STATIC);
    //rbCube->GetRigidBody()->setUserData(rbCube);
    reactphysics3d::Vector3 scalebox = GetScaleBox(path);
   
    // collision
    BoxShape* boxShape = _reactPhysic->GetPhysicCommon()->createBoxShape(scalebox);
    
    CollisionComponentCreation(building, rbCube, boxShape, nullTransform);

    actors.emplace_back(building);
}
     
void TestScene::ActorCreation()
{
    //Create actor and their components
    reactphysics3d::Transform nullTransform(reactphysics3d::Vector3(0, 0, 0), reactphysics3d::Quaternion::identity());

    /*
    //Create Helmet
    Helmet* helmet1 = new Helmet(Init, m_BackgroundMode, m_pRenderPass, "helmet1");
    helmet1->setPosition(float3(0, 2, -5));
    reactphysics3d::Transform helmetTransform(reactphysics3d::Vector3(0,2,-5), reactphysics3d::Quaternion::identity());
    
    //rb
    RigidbodyComponent* helmetRigidbody = RigidbodyComponentCreation(helmet1, helmetTransform, BodyType::DYNAMIC);

    //collision
    SphereShape* sphereShape = _reactPhysic->GetPhysicCommon()->createSphereShape(0.5);
    CollisionComponentCreation(helmet1, helmetRigidbody, sphereShape, nullTransform);
    */
    

    /*
    //Create a plane
    Plane* plane1 = new Plane(Init, m_BackgroundMode, m_pRenderPass, "plane1");
    reactphysics3d::Transform planeTransform(reactphysics3d::Vector3(0, -4, 0), reactphysics3d::Quaternion::identity());

    //rb
    RigidbodyComponent* rbPlane = RigidbodyComponentCreation(plane1, planeTransform, BodyType::STATIC);

    //collision
    BoxShape* boxShape = _reactPhysic->GetPhysicCommon()->createBoxShape(reactphysics3d::Vector3(250, 2, 250));
    CollisionComponentCreation(plane1, rbPlane, boxShape, nullTransform);

    */
    

    //Add actor to list
    //actors.emplace_back(helmet1);
    //actors.emplace_back(plane1);   
}

RigidbodyComponent* TestScene::RigidbodyComponentCreation(Actor* actor, reactphysics3d::Transform transform, BodyType type)
{
    RigidbodyComponent* rigidbody = new RigidbodyComponent(actor->GetActor(), transform, _reactPhysic->GetPhysicWorld());
    rigidbody->GetRigidBody()->setType(type);
    actor->addComponent(rigidbody);
    return rigidbody;
}

void TestScene::CollisionComponentCreation(Actor* actor, RigidbodyComponent* rb, CollisionShape* shape, reactphysics3d::Transform transform)
{
    CollisionComponent* colisionComponent = new CollisionComponent(actor->GetActor(), shape);
    colisionComponent->SetCollisionShape(shape);
    colisionComponent->SetCollider(rb->GetRigidBody()->addCollider(shape, transform));
    actor->addComponent(colisionComponent);
    colisionComponent->GetCollider()->getMaterial().setBounciness(0);
}

// Render a frame
void TestScene::Render()
{
    auto* pFramebuffer = GetCurrentFramebuffer();

    ambientlight->CreateSRB(ColorBuffer, DepthZBuffer);
    for (auto light : lights)
    {
        light->CreateSRB(ColorBuffer, DepthZBuffer);
    }

    BeginRenderPassAttribs RPBeginInfo;
    RPBeginInfo.pRenderPass  = m_pRenderPass;
    RPBeginInfo.pFramebuffer = pFramebuffer;

    OptimizedClearValue ClearValues[4];
    // Color
    ClearValues[0].Color[0] = 0.f;
    ClearValues[0].Color[1] = 0.f;
    ClearValues[0].Color[2] = 0.f;
    ClearValues[0].Color[3] = 0.f;

    // Depth Z
    ClearValues[1].Color[0] = 1.f;
    ClearValues[1].Color[1] = 1.f;
    ClearValues[1].Color[2] = 1.f;
    ClearValues[1].Color[3] = 1.f;

    // Depth buffer
    ClearValues[2].DepthStencil.Depth = 1.f;

    // Final color buffer
    ClearValues[3].Color[0] = 0.0625f;
    ClearValues[3].Color[1] = 0.0625f;
    ClearValues[3].Color[2] = 0.0625f;
    ClearValues[3].Color[3] = 0.f;

    RPBeginInfo.pClearValues        = ClearValues;
    RPBeginInfo.ClearValueCount     = _countof(ClearValues);
    RPBeginInfo.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    m_pImmediateContext->BeginRenderPass(RPBeginInfo);

    for (auto actor : actors)
    {
        if (actor->getState() == Actor::ActorState::Active)
        {
            actor->RenderActor(*_player->GetCamera(), false);
        }
    }

    m_pImmediateContext->NextSubpass();

    envMaps->RenderActor(*_player->GetCamera(), false);

    ambientlight->RenderActor(*_player->GetCamera(), false);
    for (auto light : lights)
    {
        light->RenderActor(*_player->GetCamera(), false);
    }

    m_pImmediateContext->EndRenderPass();

    if (m_pDevice->GetDeviceCaps().IsGLDevice())
    {
        // In OpenGL we now have to copy our off-screen buffer to the default framebuffer
        auto* pOffscreenRenderTarget = pFramebuffer->GetDesc().ppAttachments[3]->GetTexture();
        auto* pBackBuffer            = m_pSwapChain->GetCurrentBackBufferRTV()->GetTexture();

        CopyTextureAttribs CopyAttribs{pOffscreenRenderTarget, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                       pBackBuffer, RESOURCE_STATE_TRANSITION_MODE_TRANSITION};
        m_pImmediateContext->CopyTexture(CopyAttribs);
    }   
}

void TestScene::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    //React physic
    _reactPhysic->Update();
    _player->UpdatePlayer(CurrTime, ElapsedTime, m_InputController);

    // Shoot
    {
        const auto& mouseState = m_InputController.GetMouseState();

        if (mouseState.ButtonFlags == MouseState::BUTTON_FLAG_RIGHT && (m_LastMouseState.ButtonFlags != mouseState.ButtonFlags))
        {
            //preparation
            float winHeight = (float)m_pSwapChain->GetDesc().Height;
            float winWidth  = (float)m_pSwapChain->GetDesc().Width;
            //Viewport Space
            float3 mousePos = float4(mouseState.PosX, mouseState.PosY, 1, 1);
            //normalised device Space
            mousePos.x = ((2 * mousePos.x) / winWidth - 1.0f) / 2;
            mousePos.y = (1.0f - (2 * mousePos.y) / winHeight) / 2;
            //homogenous Space
            float4 clipPos = float4(mousePos.x, mousePos.y, -1.0f, 1.0f);
            //camera Space

            float4x4 camProj = _player->GetCamera()->GetProjMatrix();
            camProj          = camProj.Transpose();
            camProj          = camProj.Inverse();


            float4 eyePos = camProj * clipPos;
            eyePos        = float4(eyePos.x, eyePos.y, 1.0f, 0.0f);
            //world Space
            float4x4 viewMatrix = _player->GetCamera()->GetViewMatrix();

            viewMatrix = viewMatrix.Transpose();
            viewMatrix = viewMatrix.Inverse();

            float4 pos4 = viewMatrix * eyePos;
            float3 pos3 = float3(pos4);
            pos3        = normalize(pos3);
            pos4        = normalize(pos4);
            //Raycast start and end

            reactphysics3d::Vector3 vec3Start = reactphysics3d::Vector3(_player->GetCamera()->GetPos().x, _player->GetCamera()->GetPos().y, _player->GetCamera()->GetPos().z);
            float3                  float3End = _player->GetCamera()->GetPos() + pos3 * 50;
            reactphysics3d::Vector3 vec3End   = reactphysics3d::Vector3(float3End.x, float3End.y, float3End.z);



            MyRaycastCallback testasr;

            Raycast interRay(vec3Start, vec3End);
            _reactPhysic->GetPhysicWorld()->raycast(interRay.GetRay(), &testasr);
            //printing
            //string message = "Start Ray = " + std::to_string(vec3Start.x) + "," + std::to_string(vec3Start.y) + "," + std::to_string(vec3Start.z);
            //Diligent::Log::Instance().addInfo(message);
            //message = "End Ray = " + std::to_string(vec3End.x) + "," + std::to_string(vec3End.y) + "," + std::to_string(vec3End.z);
            //Diligent::Log::Instance().addInfo(message);
        }
    }
    m_LastMouseState = m_InputController.GetMouseState();
    
    
    //Draw log
    Diligent::Log::Instance().Draw();

    // Animate Actors
    for (auto actor : actors)
    {
            actor->Update(CurrTime, ElapsedTime);
    }

    for (auto light : lights)
    {
        light->UpdateActor(CurrTime, ElapsedTime);
    }
}

void TestScene::addActor(Actor* actor)
{
    actors.emplace_back(actor);
    actor->Initialize(Init);
}

void TestScene::removeActor(Actor* actor)
{
    auto iter = std::find(begin(actors), end(actors), actor);
    if (iter != end(actors))
    {
        std::iter_swap(iter, end(actors) - 1);
        actors.pop_back();
    }
}

void TestScene::addRay(float3 beginPoint, float3 endPoint, float detail, float scale)
{
    for (float i = 0; i <= detail; i++)
    {
        float x(beginPoint.x + (endPoint.x - beginPoint.x) * (i / detail)), 
              y(beginPoint.y + (endPoint.y - beginPoint.y) * (i / detail)), 
              z(beginPoint.z + (endPoint.z - beginPoint.z) * (i / detail));
        Ray* ray = new Ray(Init, m_BackgroundMode, m_pRenderPass);
        ray->setPosition(float3(x, y, z));
        ray->setScale(scale);
        addActor(ray);
    }
}

void TestScene::SetLastActorTransform(float3 _coord, Quaternion _quat, float _scale) {
    Actor* actor = actors.back();
    actor->setPosition(_coord);
    actor->setRotation(_quat);
    actor->setScale(_scale);
}


void TestScene::CreateTargetAndLight() {

    PointLight* light1 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light1->setLocation(float3(0.3, -1.8, -12));
    light1->setColor(float3(1, 0, 0));
    lights.emplace_back(light1);

    PointLight* light2 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light2->setLocation(float3(5, -1.4, -22.25));
    light2->setColor(float3(1, 0, 0));
    lights.emplace_back(light2);

    PointLight* light3 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light3->setLocation(float3(15.3, -1.4, -13.8));
    light3->setColor(float3(1, 0, 0));
    lights.emplace_back(light3);


    PointLight* light4 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light4->setLocation(float3(16.3, -1.4, -0.5));
    light4->setColor(float3(1, 0, 0));
    lights.emplace_back(light4);


    PointLight* light5 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light5->setLocation(float3(12, -1.4, 12));
    light5->setColor(float3(1, 0, 0));
    lights.emplace_back(light5);

    PointLight* light6 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light6->setLocation(float3(-10, -1.8, 16.5));
    light6->setColor(float3(1, 0, 0));
    lights.emplace_back(light6);

    PointLight* light7 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light7->setLocation(float3(-24.8, -1.4, 11));
    light7->setColor(float3(1, 0, 0));
    lights.emplace_back(light7);

    PointLight* light8 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light8->setLocation(float3(-38, -1.4, 2));
    light8->setColor(float3(1, 0, 0));
    lights.emplace_back(light8);

    PointLight* light9 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light9->setLocation(float3(-31.8, -1.4, -10));
    light9->setColor(float3(1, 0, 0));
    lights.emplace_back(light9);

    PointLight* light10 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light10->setLocation(float3(-31.8, -1.4, -20));
    light10->setColor(float3(1, 0, 0));
    lights.emplace_back(light10);

    PointLight* light11 = new PointLight(Init, m_pRenderPass, pShaderSourceFactory);
    light11->setLocation(float3(-42.9, -1.4, -32));
    light11->setColor(float3(0, 1, 0));
    light11->setSize(2);
    lights.emplace_back(light11);

    reactphysics3d::Transform nullTransform(reactphysics3d::Vector3(0, 0, 0), reactphysics3d::Quaternion::identity());
    SphereShape*              sphereShape = _reactPhysic->GetPhysicCommon()->createSphereShape(1);

    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target1");

        target->setPosition(float3(2, 3, -27.8));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(2, -2, -20), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(0.5 * PI), 0, 0, sin(0.5 * PI)));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target2");

        target->setPosition(float3(16, 3, -17));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(21, 4, -22), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(140 * PI / 180), 0, sin(140 * PI / 180), 0));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target6");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(22.5, 2.5, -6.5), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(-20 * PI / 180), 0, sin(-20 * PI / 180), 0));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target3");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(0, -2, 2.5), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(90 * PI / 180), 0, sin(90 * PI / 180), 0));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target4");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(3, -2, 15), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target5");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(2, 5, 23), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target7");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(-19, 3.5, 6.5), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(90 * PI / 180), 0, sin(90 * PI / 180), 0));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target8");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(-27, 3.5, 0), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(-45 * PI / 180), 0, sin(-45 * PI / 180), 0));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target9");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(-45, 4, 3), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(45 * PI / 180), 0, sin(45 * PI / 180), 0));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
    {
        Target* target = new Target(Init, m_BackgroundMode, m_pRenderPass, "target10");

        target->setPosition(float3(0, 2, -5));
        reactphysics3d::Transform targetTransform(reactphysics3d::Vector3(-33, -2, -14.5), reactphysics3d::Quaternion::identity());
        target->setScale(0.33);
        target->setRotation(Quaternion(cos(90 * PI / 180), 0, sin(90 * PI / 180), 0));

        //rb
        RigidbodyComponent* targetRigidbody = RigidbodyComponentCreation(target, targetTransform, BodyType::STATIC);

        //collision
        CollisionComponentCreation(target, targetRigidbody, sphereShape, nullTransform);
        targets.emplace_back(target);
        actors.emplace_back(target);
    }
}
   
reactphysics3d::Vector3 TestScene::GetScaleBox(const char* path)
{
    if (!strcmp(path, "models\\Immeubles\\v2test\\AssetAntenne.gltf"))
    {

        return reactphysics3d::Vector3(0.1, 1.5, 0.1);
    }

    if (!strcmp(path,"models\\Immeubles\\v2test\\AssetToit.gltf"))
    {

        return reactphysics3d::Vector3(1, 1, 1.43);
    }

    if (!strcmp(path,"models\\Immeubles\\v2test\\AssetToit2.gltf"))
    {

        return reactphysics3d::Vector3(1.14, 1.3, 1.9);
    }

    if (!strcmp(path, "models\\Immeubles\\v2test\\AssetTuyau.gltf"))
    {

        return reactphysics3d::Vector3(0.33, 0.33, 1.5);
    }

    if (!strcmp(path,"models\\Immeubles\\v2test\\Immeuble1_V2.gltf"))
    {
        return reactphysics3d::Vector3(3, 6.2,7);
    }

    if (!strcmp(path,"models\\Immeubles\\v2test\\Immeuble2_V3.gltf"))
    {

        return reactphysics3d::Vector3(3.753, 2.773, 6);
    }

    if (!strcmp(path, "models\\Immeubles\\v2test\\Immeuble3_X2_V2.gltf"))
    {

        return reactphysics3d::Vector3(5.8, 33, 5.8);
    }

    return reactphysics3d::Vector3(1, 1, 1);
}        
   

} // namespace Diligent
