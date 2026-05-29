#pragma once
#include <string>
#include <vector>

/**
 * @class ConfigManager
 * @brief Static utility class for managing application configuration via a local text file.
 * * Handles reading and writing user preferences like video paths, FPS, and rendering strategies.
 */
class ConfigManager
{
private:
    inline static const std::string CONFIG_FILE = "settings.conf";
    inline static const std::vector<std::string> allowed = {".mp4", ".mov", ".avi"};

public:
    /**
     * @brief Retrieves a specific configuration value by its key.
     * @param key The setting key to search for (e.g., "target_fps").
     * @return The value associated with the key, or an empty string if not found.
     */
    static std::string getValFromSettings(const std::string &key);

    /**
     * @brief Saves or updates a key-value pair in the configuration file.
     * @param key The setting key to save.
     * @param value The value to associate with the key.
     * @return true if the setting was successfully saved, false otherwise.
     */
    static bool setValToSettings(const std::string &key, const std::string &value);

    /**
     * @brief Checks if the provided file extension is supported by the engine.
     * @param extension The file extension including the dot (e.g., ".mp4").
     * @return true if the format is supported.
     */
    static bool isSupportedVideoFormat(const std::string &extension);

    /**
     * @brief Validates if the given file path exists, is a regular file, and has a supported format.
     * @param path Absolute or relative path to the video file.
     * @return true if the file is a valid and supported video.
     */
    static bool isValidVideoFile(const std::string &path);

    /**
     * @brief Loads the last used video path from the settings file.
     * @return The validated file path, or an empty string if invalid/missing.
     */
    static std::string loadVideoPath();

    /**
     * @brief Saves the video path to settings. Handles the special "webcam" keyword.
     * @param path The path to save, or "webcam" to switch to live feed mode.
     * @return true if successfully saved.
     */
    static bool saveVideoPath(const std::string &path);

    /**
     * @brief Checks if the user is currently set to use the webcam.
     * @return true if webcam mode is enabled.
     */
    static bool GetUseWebcam();

    /**
     * @brief Saves the character used for filling edge detection areas.
     * @param character A single-character string.
     * @return true if saved successfully, false if the string length is not exactly 1.
     */
    static bool saveFillChar(const std::string &character);
};