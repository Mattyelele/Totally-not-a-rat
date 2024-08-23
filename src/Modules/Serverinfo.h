// ServerInfo.h
#pragma once
#include <string>

// Define the ServerInfo struct
struct ServerInfo {
    std::string name;       // Server name
    std::string map;        // Map name
    int currentPlayers;     // Current number of players
    int maxPlayers;         // Maximum number of players
    std::string ip;
    std::string port;

    // Constructor for easy initialization
    ServerInfo(const std::string& name, const std::string& map, int currentPlayers, int maxPlayers, const std::string& ip, const std::string& port)
        : name(name), map(map), currentPlayers(currentPlayers), maxPlayers(maxPlayers), ip(ip), port(port) {}
};
