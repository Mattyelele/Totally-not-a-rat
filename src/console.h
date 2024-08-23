// console.h
#pragma once
#include <imgui/imgui.h>
#include <vector>
#include <string>

class Console {
public:
    Console();
    void Clear();
    void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    void Draw(const char* title, bool* p_open = nullptr);
    void SetAutoScroll(bool autoScroll) { AutoScroll = autoScroll; }

private:
    std::vector<std::string> Items;
    bool AutoScroll;
};