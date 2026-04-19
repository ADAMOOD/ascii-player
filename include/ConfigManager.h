#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

class ConfigManager
{
private:
    inline static const std::string CONFIG_FILE = "settings.conf";

public:
static std::string loadVideoPath() {
    if (!std::filesystem::exists(CONFIG_FILE)) {
        return ""; 
    }

    std::ifstream fReader(CONFIG_FILE);
    std::string line;
    
    while (std::getline(fReader, line)) {
        if (line.empty()) continue;
        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            if (key == "video_path") {
                return value;
            }
        }
    }

    return ""; 
}

    static bool saveVideoPath(const std::string &path)
    {
        std::ofstream fWriter(CONFIG_FILE);

        if (fWriter.is_open())
        {
            fWriter << path; 
            fWriter.close();
            return true;
        }

        return false; 
    }
};