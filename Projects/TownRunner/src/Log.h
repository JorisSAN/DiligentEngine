#pragma once



#include <fstream>
#include <string>
#include <vector>
#include "imgui.h"

using std::string;

namespace Diligent
{
class Log
{
public:
    Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

//    void Initialize();
    void addInfo(const string message);
    void        clear();
    void Draw();
    void save();
    void        setAutoSave(bool varAutoSave) { autosave = varAutoSave; }

    private:
    std::vector<std::string> logs;
    bool autosave;
};
} // namespace Diligent
