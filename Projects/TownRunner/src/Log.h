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
    static Log& Instance();
    //Log(const Log&) = delete;
    //Log& operator=(const Log&) = delete;

    //New singleton functions
    Log& operator=(const Log&) {}
    Log(const Log&) {}

//    void Initialize();
    void addInfo(const string message);
    void        clear();
    void Draw();
    void save();
    void        setAutoSave(bool varAutoSave) { autosave = varAutoSave; }

    private:
    static Log               m_instance;
    std::vector<std::string> logs;
    bool autosave;
};
} // namespace Diligent
