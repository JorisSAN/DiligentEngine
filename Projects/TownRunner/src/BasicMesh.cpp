#include "BasicMesh.h"

#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"

using namespace Diligent;

BasicMesh::BasicMesh(const SampleInitInfo& InitInfo, const char* path, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass)
{
    GLTFObject::Initialize(InitInfo, RenderPass);
    pathName = path;
    m_BackgroundMode = backGround;
    setObjectPath(path);
    _actorType = ActorType::BasicMesh;
}
void BasicMesh::UpdateActor(double CurrTime, double ElapsedTime)
{
    // Animate the cube
    //setRotation(Quaternion::RotationFromAxisAngle(float3(0, 1, 0), static_cast<float>(CurrTime) * 1.0f));
    //    m_WorldMatrix = float4x4::Translation(coord[0], coord[1], coord[2]);
    GLTFObject::computeWorldTransform();
}
