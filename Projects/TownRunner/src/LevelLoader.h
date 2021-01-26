#pragma once

#include <iostream>
#include <fstream>

#include <typeinfo>
#include "TestScene.hpp"
namespace Diligent
{


void ReadFile(std::string fileName, const SampleInitInfo& InitInfo,TestScene* scene)
{

    std::ifstream file(fileName.c_str());

    if (!file)
    {

        return;
    }
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream ss(line);

        std::stringstream test(line);
        std::string       actorClass;
        std::getline(test, actorClass, '/');
        std::string xCoord;
        std::string yCoord;
        std::string zCoord;
        std::getline(test, xCoord, ',');
        std::getline(test, yCoord, ',');
        std::getline(test, zCoord, '/');
        std::string xQuat;
        std::string yQuat;
        std::string zQuat;
        std::string wQuat;
        std::string scale;
        std::getline(test, xQuat, ',');
        std::getline(test, yQuat, ',');
        std::getline(test, zQuat, ',');
        std::getline(test, wQuat, '/');
        std::getline(test, scale, ',');
        //float3 coord = float3(std::stof(xCoord.c_str()), std::stof(yCoord.c_str()), std::stof(zCoord.c_str()));
        float3     coord = float3(std::stof(xCoord), std::stof(yCoord), std::stof(zCoord));
        Quaternion quat  = Quaternion(std::stof(xQuat), std::stof(yQuat), std::stof(zQuat), std::stof(wQuat));
        if (actorClass == "BasicMesh")
        {
            std::string objPath;

            std::getline(test, objPath, ',');
            scene->CreateBasicMesh(objPath.c_str(), InitInfo,coord);
        }
        else
        {
            scene->CreateAdaptedActor(actorClass, InitInfo);
        }
        //set position quat and scale

        scene->SetLastActorTransform(coord, quat, std::stof(scale));
    }

    int isfserfs = 0;
    isfserfs++;

    file.close();
    //    transforms.push_back(float3(2, 0, 0));
    //    transforms.push_back(float3(4, 0, 0));
}


} // namespace Diligent