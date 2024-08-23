#include "console.h"
#include <stdarg.h>

Console::Console() : AutoScroll(true) {
    Clear();
}

void Console::Clear() {
    Items.clear();
}

void Console::AddLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    buf[sizeof(buf) - 1] = 0;
    va_end(args);
    Items.push_back(std::string(buf));
}

void Console::Draw(const char* title, bool* p_open) {
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) Clear();
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &AutoScroll);

    ImGui::Separator();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& item : Items) {
        ImGui::TextUnformatted(item.c_str());
    }

    if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}