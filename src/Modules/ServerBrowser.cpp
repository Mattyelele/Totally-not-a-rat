// Serverbrowser.cpp
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include "Serverinfo.h"
#include "Memory.h"

using json = nlohmann::json;

std::vector<ServerInfo> servers;
void RenderServerBrowser(const std::vector<ServerInfo>& servers);

void Plsworkservers() {
    RenderServerBrowser(servers);
}

void runPythonScript() {
    std::string command = "python scraper.py";
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to execute Python script." << std::endl;
    }
}

void loadServerData() {
    try {
    std::string filePath = "server_data.json";
    std::ifstream inputFile(filePath);

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open " << filePath << std::endl;
        return;
    }

    json scrapedData;
    try {
        inputFile >> scrapedData;
    }
    catch (const json::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    std::string playerCount = scrapedData.value("player_count", "N/A");
    std::string serverCount = scrapedData.value("server_count", "N/A");

    ImGui::Text("Player Count: %s", playerCount.c_str());
    ImGui::Text("Server Count: %s", serverCount.c_str());

    servers.clear();

    for (const auto& server : scrapedData["servers"]) {
        std::string name = server.value("name", "N/A");
        std::string map = server.value("map", "N/A");
        int currentPlayers = server.value("currentPlayers", 0);
        int maxPlayers = server.value("maxPlayers", 0);
        std::string ip = server.value("ip", "N/A");
        std::string port = server.value("port", "N/A");

        servers.emplace_back(name, map, currentPlayers, maxPlayers, ip, port);
    }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}

bool SetClipBoard(const std::wstring& text) {
    if (!OpenClipboard(nullptr)) {
        std::cout << "Failed to open clipboard." << std::endl;
        return false;
    }

    size_t size = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hGlob) {
        std::cout << "Failed to allocate global memory" << std::endl;
        CloseClipboard();
        return false;
    }

    memcpy(GlobalLock(hGlob), text.c_str(), size);
    GlobalUnlock(hGlob);

    if (!SetClipboardData(CF_UNICODETEXT, hGlob)) {
        std::cout << "Failed to set clipboard data." << std::endl;
        GlobalFree(hGlob);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

void OnJoinButtonClick(const std::string& serverIP) {
    std::wstring command = L"connect " + std::wstring(serverIP.begin(), serverIP.end());

    if (SetClipBoard(command)) {
        std::cout << "Command copied to clipboard: " << std::string(command.begin(), command.end()) << std::endl;
    }
    else {
        std::cerr << "Failed to copy command to clipboard." << std::endl;
    }
}

void RenderServerBrowser(const std::vector<ServerInfo>& servers) {
    ImGui::Begin("Server Browser");

    if (ImGui::Button("Scrap Site")) {
        runPythonScript();
    }

    if (ImGui::Button("Refresh")) {
        loadServerData();
    }

    static char filter[128] = "";
    ImGui::InputText("Filter", filter, IM_ARRAYSIZE(filter));

    ImGui::Separator();

    ImGui::Columns(6, "serverBrowserColumns"); // Set up columns for IP, Port, Name, Map, Players
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Map"); ImGui::NextColumn();
    ImGui::Text("Players"); ImGui::NextColumn();
    ImGui::Text("IP"); ImGui::NextColumn();
    ImGui::Text("Port"); ImGui::NextColumn();
    ImGui::Text("Join"); ImGui::NextColumn();
    ImGui::Separator();

    for (const auto& server : servers) {
        if (filter[0] != '\0' && !strstr(server.name.c_str(), filter)) {
            continue;  // Skip servers that do not match filter
        }

        ImGui::Text("%s", server.name.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", server.map.c_str()); ImGui::NextColumn();
        ImGui::Text("%d/%d", server.currentPlayers, server.maxPlayers); ImGui::NextColumn();
        ImGui::Text("%s", server.ip.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", server.port.c_str()); ImGui::NextColumn();

        // Button to join server
        if (ImGui::Button("Join")) {
            std::string serverIP = server.ip + ":" + server.port;
            OnJoinButtonClick(serverIP);
        }; ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::End();
}

