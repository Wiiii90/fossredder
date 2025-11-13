#pragma once
#include <string>
#include <filesystem>

namespace env {
    // Load key=value pairs from a .env file into process environment.
    // - overwrite: if true, replaces existing env vars, otherwise leaves them.
    bool load_dotenv(const std::string& path, bool overwrite = false);

    // Get an environment variable as string or default if missing.
    std::string get(const std::string& key, const std::string& def = "");

    // Return a per-user cache directory for the application and ensure it exists.
    // Platform-specific locations:
    //  - Windows: %LOCALAPPDATA%/<appName>/cache
    //  - macOS: ~/Library/Caches/<appName>
    //  - Linux: $XDG_CACHE_HOME/<appName> or ~/.cache/<appName>
    //  - Fallback: current working directory / <appName>
    std::filesystem::path getUserCacheDir(const std::string& appName = "FOSSredder");
}
