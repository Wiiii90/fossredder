#include "debug/pch.h"
#include "debug/FileDebugger.h"
#include <nlohmann/json.hpp>
#include <fstream>

static std::string make_session_dir(const std::string& base) {
    try {
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#if defined(_WIN32)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        char buf[128];
        std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
        std::string session = std::string(buf);
        std::filesystem::path p(base);
        p /= session;
        std::filesystem::create_directories(p);
        return p.string();
    } catch (...) {
        return base;
    }
}

FileDebugger::FileDebugger(const std::string& baseDir) : baseDir_(baseDir) {
    try {
        std::filesystem::create_directories(baseDir_);
    } catch (...) {}
    // create a per-run session subdirectory so debug artifacts don't collide
    try {
        std::string sess = make_session_dir(baseDir_);
        baseDir_ = sess;
    } catch (...) {}
}

FileDebugger::~FileDebugger() {
    flush();
}

static std::filesystem::path safe_join(const std::filesystem::path& base, const std::string& rel) {
    std::filesystem::path p(rel);
    std::filesystem::path clean;
    for (const auto& part : p) {
        if (part == "..") continue;
        if (part == "/" || part == "\\") continue;
        clean /= part;
    }
    return base / clean;
}

void FileDebugger::writeText(const std::string& relPath, const std::string& text) {
    std::lock_guard<std::mutex> g(mtx_);
    auto outp = safe_join(baseDir_, relPath);
    try {
        std::filesystem::create_directories(outp.parent_path());
        std::string outText = text;
        bool tryPretty = false;
        if (outp.has_extension() && outp.extension() == ".json") tryPretty = true;
        else {
            auto pos = text.find_first_not_of(" \t\r\n");
            if (pos != std::string::npos) {
                char c = text[pos];
                if (c == '{' || c == '[') tryPretty = true;
            }
        }

        if (tryPretty) {
            try {
                nlohmann::json j = nlohmann::json::parse(text);
                outText = j.dump(4);
            } catch (...) {
            }
        }

        std::ofstream ofs(outp, std::ios::binary);
        if (ofs) ofs << outText;
    } catch (...) {}
}

void FileDebugger::writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> g(mtx_);
    auto outp = safe_join(baseDir_, relPath);
    try {
        std::filesystem::create_directories(outp.parent_path());
        std::ofstream ofs(outp, std::ios::binary);
        if (ofs) ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    } catch (...) {}
}

void FileDebugger::flush() {
}
