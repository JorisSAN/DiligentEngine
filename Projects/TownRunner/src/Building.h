#pragma once
#ifndef BUILDING_H
#define BUILDING_H

#include "GLTFObject.h"

namespace Diligent
{

class Building : public GLTFObject
{
public:
    Building(const SampleInitInfo& InitInfo, BackgroundMode backGround, RefCntAutoPtr<IRenderPass>& RenderPass, std::string name, const char* path);

    void UpdatePlayer(double CurrTime, double ElapsedTime, InputController& Controller);

private:
    std::string pathName;


};

}

#endif