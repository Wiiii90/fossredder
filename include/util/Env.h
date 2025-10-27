#pragma once
#include <string>

namespace env {
    // Load key=value pairs from a .env file into process environment.
    // - overwrite: if true, replaces existing env vars, otherwise leaves them.
    bool load_dotenv(const std::string& path, bool overwrite = false);

    // Get an environment variable as string or default if missing.
    std::string get(const std::string& key, const std::string& def = "");
}
