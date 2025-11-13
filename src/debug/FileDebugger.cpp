#include "pch.h"
#include "debug/FileDebugger.h"
#include <fstream>
#include <nlohmann/json.hpp>

FileDebugger::FileDebugger(const std::string& baseDir) : baseDir_(baseDir) {
    try {
        std::filesystem::create_directories(baseDir_);
    } catch (...) {}
}

FileDebugger::~FileDebugger() {
    flush();
}

static std::filesystem::path safe_join(const std::filesystem::path& base, const std::string& rel) {
    std::filesystem::path p(rel);
    // sanitize: remove absolute paths and .. components
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
        // If filename ends with .json, prefer pretty printing
        if (outp.has_extension() && outp.extension() == ".json") tryPretty = true;
        else {
            // quick heuristic: leading non-space char is '{' or '['
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
                // parsing failed, fall back to raw text
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
    // no-op for now
}
