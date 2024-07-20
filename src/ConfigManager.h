#pragma once

#include <string>

class ConfigManager {
public:
    static bool LoadConfig(int& openKey);
    static bool SaveConfig(int openKey);
};
