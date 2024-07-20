#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

const std::string CONFIG_FILE = "config.txt";

bool ConfigManager::LoadConfig(int& openKey) {
    std::ifstream file(CONFIG_FILE);
    if (!file.is_open()) {
        return false; // File not found or cannot be opened
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        int value;
        if (std::getline(iss, key, '=') && (iss >> value)) {
            if (key == "OpenKey") {
                openKey = value;
                file.close();
                return true;
            }
        }
    }

    file.close();
    return false; // Key not found in file
}

bool ConfigManager::SaveConfig(int openKey) {
    std::ofstream file(CONFIG_FILE);
    if (!file.is_open()) {
        return false; // File cannot be opened
    }

    file << "OpenKey=" << openKey << std::endl;
    file.close();
    return true;
}
