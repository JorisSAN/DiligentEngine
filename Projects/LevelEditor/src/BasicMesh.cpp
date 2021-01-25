#include "BasicMesh.h"
#include "TexturedCube.hpp"

#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "TexturedCube.hpp"

using namespace Diligent;

BasicMesh::BasicMesh(const SampleInitInfo& InitInfo, const char* path)
{
    GLTFObject::Initialize(InitInfo);
    pathName = path;
    setObjectPath(path);
}
void BasicMesh::UpdateActor(double CurrTime, double ElapsedTime)
{
    // Animate the cube
    //setRotation(Quaternion::RotationFromAxisAngle(float3(0, 1, 0), static_cast<float>(CurrTime) * 1.0f));
    //    m_WorldMatrix = float4x4::Translation(coord[0], coord[1], coord[2]);
    GLTFObject::computeWorldTransform();
}
