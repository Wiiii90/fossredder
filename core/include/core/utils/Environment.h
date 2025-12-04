#pragma once

#include <string>
#include <filesystem>

namespace env {
    bool load_dotenv(const std::string& path, bool overwrite = false);
    std::string get(const std::string& key, const std::string& def = "");
    std::filesystem::path getUserCacheDir(const std::string& appName = "FOSSredder");

    // Note: runtime deployment/fallback is handled by build/deploy scripts (CMake). No runtime fallbacks declared here.
}
