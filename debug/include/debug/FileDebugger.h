#pragma once

#include "debug/IDebugger.h"
#include <string>
#include <vector>
#include <mutex>

class FileDebugger : public IDebugger {
public:
    // baseOrProcess: if second arg empty, this is treated as processName and base dir is ./debug_output
    explicit FileDebugger(const std::string& baseOrProcess, const std::string& processName = "");
    ~FileDebugger() override;

    bool enabled() const override { return true; }
    void writeText(const std::string& relPath, const std::string& text) override;
    void writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) override;
    void flush() override;

private:
    std::string baseDir_; // actual session directory used for writes
    std::string processName_;
    std::mutex mtx_;
};
