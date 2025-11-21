#pragma once

#include "debug/IDebugger.h"
#include <string>
#include <vector>
#include <mutex>

class FileDebugger : public IDebugger {
public:
    explicit FileDebugger(const std::string& baseDir);
    ~FileDebugger() override;

    bool enabled() const override { return true; }
    void writeText(const std::string& relPath, const std::string& text) override;
    void writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) override;
    void flush() override;

private:
    std::string baseDir_;
    std::mutex mtx_;
};
