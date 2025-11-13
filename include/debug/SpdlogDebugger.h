#pragma once

#include "debug/IDebugger.h"
#include <memory>
#include <string>
#include <spdlog/logger.h>

class SpdlogDebugger : public IDebugger {
public:
    // backend can be used for binary writes (FileDebugger)
    SpdlogDebugger(const std::string& loggerName, std::shared_ptr<IDebugger> backend = nullptr);
    ~SpdlogDebugger() override;

    bool enabled() const override;
    void writeText(const std::string& relPath, const std::string& text) override;
    void writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) override;
    void flush() override;

private:
    std::shared_ptr<spdlog::logger> m_logger_;
    std::shared_ptr<IDebugger> m_backend_;
    bool m_enabled_;
};
