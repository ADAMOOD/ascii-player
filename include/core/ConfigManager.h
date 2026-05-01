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
    static std::string getValFromSettings(const std::string &key)
    {
        if (!std::filesystem::exists(CONFIG_FILE))
            return "";

        std::ifstream fReader(CONFIG_FILE);
        std::string line;
        const std::string searched = key + "=";

        while (std::getline(fReader, line))
        {
            if (line.compare(0, searched.length(), searched) == 0)
            {
                return line.substr(searched.length());
            }
        }
        return "";
    }

    static bool setValToSettings(const std::string &key, const std::string &value)
    {
        const std::string TEMP_FILE = "settings.tmp";
        std::ifstream fReader(CONFIG_FILE);
        std::ofstream fWriter(TEMP_FILE);

        if (!fWriter.is_open())
            return false;

        bool found = false;
        std::string line;
        const std::string searched = key + "=";

        if (fReader.is_open())
        {
            while (std::getline(fReader, line))
            {
                if (line.compare(0, searched.length(), searched) == 0)
                {
                    fWriter << searched << value << "\n";
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
            fWriter << searched << value << "\n";
        }

        fWriter.close();

        try
        {
            if (std::filesystem::exists(CONFIG_FILE))
            {
                std::filesystem::remove(CONFIG_FILE);
            }
            std::filesystem::rename(TEMP_FILE, CONFIG_FILE);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "[ERR] " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    static bool isSupportedVideoFormat(const std::string &extension)
    {
        return std::find(allowed.begin(), allowed.end(), extension) != allowed.end();
    }

    static bool isValidVideoFile(const std::string &path)
    {
        if (path.empty() || !std::filesystem::exists(path))
            return false;
        if (!std::filesystem::is_regular_file(path))
            return false;
        return isSupportedVideoFormat(std::filesystem::path(path).extension().string());
    }

    static std::string loadVideoPath()
    {
        std::string val = getValFromSettings("video_path");
        if (isValidVideoFile(val))
            return val;
        return "";
    }

    static bool saveVideoPath(const std::string &path)
    {
        if (isValidVideoFile(path))
        {
            return setValToSettings("video_path", path);
        }
        return false;
    }
};