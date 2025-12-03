#include "core/pch.h"
#include "core/utils/Environment.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

namespace env {
    namespace {
        static inline void trim(std::string& s) {
            auto wsfront = std::find_if_not(s.begin(), s.end(), [](unsigned char c) {return std::isspace(c); });
            auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) {return std::isspace(c); }).base();
            if (wsback <= wsfront) { s.clear(); return; }
            s = std::string(wsfront, wsback);
        }
    }

    bool load_dotenv(const std::string& path, bool overwrite) {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            if (line[0] == '#' || line.rfind("//", 0) == 0) continue;
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            trim(key);
            trim(val);
            if (key.empty()) continue;
#ifdef _WIN32
            // perform case-insensitive comparison for PATH
            std::string keyUpper = key;
            std::transform(keyUpper.begin(), keyUpper.end(), keyUpper.begin(), ::toupper);
            if (!overwrite) {
                DWORD len = GetEnvironmentVariableA(key.c_str(), nullptr, 0);
                if (len != 0) {
                    // If key is PATH, merge new entries instead of skipping/overwriting
                    if (keyUpper == "PATH") {
                        // read existing PATH
                        std::string buf(len, '\0');
                        GetEnvironmentVariableA(key.c_str(), &buf[0], len);
                        if (!buf.empty() && buf.back() == '\0') buf.pop_back();
                        std::string existing = buf;
                        if (!val.empty()) {
                            // if val already contains existing prefix, avoid duplication
                            if (existing.find(val) == std::string::npos) {
                                std::string merged = val;
                                if (!merged.empty() && merged.back() != ';') merged.push_back(';');
                                merged += existing;
                                SetEnvironmentVariableA(key.c_str(), merged.c_str());
                            }
                        }
                    }
                    continue;
                }
            }
            SetEnvironmentVariableA(key.c_str(), val.c_str());
#else
            // POSIX: merge PATH when appropriate
            std::string keyUpper = key;
            std::transform(keyUpper.begin(), keyUpper.end(), keyUpper.begin(), ::toupper);
            if (!overwrite && getenv(key.c_str()) != nullptr) {
                if (keyUpper == "PATH") {
                    const char* cur = getenv("PATH");
                    std::string existing = cur ? std::string(cur) : std::string();
                    if (existing.find(val) == std::string::npos) {
                        std::string merged = val;
                        if (!merged.empty() && merged.back() != ':') merged.push_back(':');
                        merged += existing;
                        setenv(key.c_str(), merged.c_str(), 1);
                    }
                }
                continue;
            }
            setenv(key.c_str(), val.c_str(), 1);
#endif
        }
        return true;
    }

    std::string get(const std::string& key, const std::string& def) {
    #ifdef _WIN32
        DWORD len = GetEnvironmentVariableA(key.c_str(), nullptr, 0);
        if (len == 0) return def;
        std::string buf(len, '\0');
        GetEnvironmentVariableA(key.c_str(), &buf[0], len);
        if (!buf.empty() && buf.back() == '\0') buf.pop_back();
        return buf;
    #else
        const char* v = getenv(key.c_str());
        return v ? std::string(v) : def;
    #endif
    }

    std::filesystem::path getUserCacheDir(const std::string& appName) {
    #if defined(_WIN32) || defined(_WIN64)
        const char* localApp = std::getenv("LOCALAPPDATA");
        if (localApp && *localApp) {
            std::filesystem::path p = std::filesystem::path(localApp) / appName / "cache";
            try { std::filesystem::create_directories(p); }
            catch (...) {}
            return p;
        }
        const char* appdata = std::getenv("APPDATA");
        if (appdata && *appdata) {
            std::filesystem::path p = std::filesystem::path(appdata) / appName / "cache";
            try { std::filesystem::create_directories(p); }
            catch (...) {}
            return p;
        }
        return std::filesystem::current_path() / appName / "cache";
    #elif defined(__APPLE__)
        const char* home = std::getenv("HOME");
        if (home && *home) {
            std::filesystem::path p = std::filesystem::path(home) / "Library" / "Caches" / appName;
            try { std::filesystem::create_directories(p); }
            catch (...) {}
            return p;
        }
        return std::filesystem::current_path() / appName / "cache";
    #else
        const char* xdg = std::getenv("XDG_CACHE_HOME");
        if (xdg && *xdg) {
            std::filesystem::path p = std::filesystem::path(xdg) / appName;
            try { std::filesystem::create_directories(p); }
            catch (...) {}
            return p;
        }
        const char* home = std::getenv("HOME");
        if (home && *home) {
            std::filesystem::path p = std::filesystem::path(home) / ".cache" / appName;
            try { std::filesystem::create_directories(p); }
            catch (...) {}
            return p;
        }
        return std::filesystem::current_path() / appName / "cache";
    #endif
    }

}
