#include "Log.h"

namespace Diligent
{
    Log Log::m_instance = Log();

    Log::Log():autosave(false) 
    {}

	void Log::addInfo(const string message) {
        logs.push_back(message);
	}

	void Log::clear() {
        logs.clear();
	}

	void Log::Draw() {
        ImGui::SetNextWindowPos(ImVec2(300, 10), ImGuiCond_FirstUseEver);

        ImGui::Begin("Log windows", nullptr, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::Button("Clear")) {
                clear();
			}
            if (ImGui::Button("Save"))
            {
                save();
            }
            ImGui::EndMenuBar();
		}
        ImGui::Checkbox("autosave ", &autosave);
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Logs");
        ImGui::BeginChild("Scrolling");
        for (auto string : logs) {
            ImGui::Text(string.c_str());
        }
        ImGui::EndChild();
        ImGui::End();
        if (autosave) {
            save();
        }
	}

    void Log::save() {
        std::ofstream file("log.txt");
        std::string   line;
        for (auto log : logs) {
            line = log;
            line += "\n";
            file << line;
        }
        file.close();
    }

    Log& Log::Instance()
    {
        return m_instance;
    }
}


