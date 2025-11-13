#pragma once

#include <string>
#include <vector>

class IDebugger {
public:
    virtual ~IDebugger() = default;
    virtual bool enabled() const = 0;
    virtual void writeText(const std::string& relPath, const std::string& text) = 0;
    virtual void writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) = 0;
    virtual void flush() {}
};
