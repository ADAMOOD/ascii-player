#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>

class ConfigManager
{
private:
    inline static const std::string CONFIG_FILE = "settings.conf";
    inline static const std::vector<std::string> allowed = {".mp4", ".mov", ".avi"};

public:
    static bool isSupportedVideoFormat(const std::string &extension)
    {
        return std::find(allowed.begin(), allowed.end(), extension) != allowed.end();
    }
    static std::string loadVideoPath()
    {
        if (!std::filesystem::exists(CONFIG_FILE))
            return "";

        std::ifstream fReader(CONFIG_FILE);
        std::string line;
        const std::string key = "video_path=";

        while (std::getline(fReader, line))
        {
            if (line.compare(0, key.length(), key) == 0)
            {
                std::string val = line.substr(key.length());
                if (isValidVideoFile(val))
                    return val;
            }
        }
        return "";
    }

    static bool saveVideoPath(const std::string &path)
    {
        const std::string TEMP_FILE = "settings.tmp";
        std::ifstream fReader(CONFIG_FILE);
        std::ofstream fWriter(TEMP_FILE);

        if (!fWriter.is_open())
            return false;

        bool found = false;
        std::string line;
        const std::string key = "video_path=";

        if (fReader.is_open())
        {
            while (std::getline(fReader, line))
            {
                if (line.compare(0, key.length(), key) == 0)
                {
                    fWriter << key << path << "\n";
                    found = true;
                }
                else if (!line.empty())
                {
                    fWriter << line << "\n";
                }
            }
            fReader.close();
        }

        if (!found)
        {
            fWriter << key << path << "\n";
        }

        fWriter.close();

        try
        {
            std::filesystem::remove(CONFIG_FILE);
            std::filesystem::rename(TEMP_FILE, CONFIG_FILE);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "[ERR] " << e.what() << std::endl;
            return false;
        }

        return true;
    }
    static bool isValidVideoFile(const std::string &path)
    {
        if (path.empty() || !std::filesystem::exists(path))
            return false;
        if (!std::filesystem::is_regular_file(path))
            return false;
        return isSupportedVideoFormat(std::filesystem::path(path).extension().string());
    }
};