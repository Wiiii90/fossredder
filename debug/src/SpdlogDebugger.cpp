#include "debug/pch.h"
#include "debug/SpdlogDebugger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ansicolor_sink.h>

SpdlogDebugger::SpdlogDebugger(const std::string& loggerName, std::shared_ptr<IDebugger> backend)
    : m_backend_(backend), m_enabled_(true)
{
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/fossredder.log", 1024 * 1024 * 5, 3);
        spdlog::sinks_init_list sinks{console_sink, file_sink};
        m_logger_ = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
        spdlog::register_logger(m_logger_);
        m_logger_->set_level(spdlog::level::debug);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
    } catch (...) {
        m_enabled_ = false;
    }
}

SpdlogDebugger::~SpdlogDebugger() {
    try {
        if (m_logger_) spdlog::drop(m_logger_->name());
    } catch (...) {}
}

bool SpdlogDebugger::enabled() const {
    return m_enabled_;
}

void SpdlogDebugger::writeText(const std::string& relPath, const std::string& text) {
    if (!m_enabled_) return;

    try {
        if (relPath.rfind("poppler/meta/", 0) == 0) {
            if (m_backend_) {
                try { m_backend_->writeText(relPath, text); } catch (...) {}
            }
            if (m_logger_) m_logger_->debug("[{}] metadata written (content omitted)", relPath);
            return;
        }
    } catch (...) {}

    if (m_logger_) m_logger_->info("[{}] {}", relPath, text);
    if (m_backend_) {
        try { m_backend_->writeText(relPath, text); } catch (...) {}
    }
}

void SpdlogDebugger::writeBytes(const std::string& relPath, const std::vector<uint8_t>& data) {
    if (!m_enabled_) return;
    if (m_backend_) {
        try {
            m_backend_->writeBytes(relPath, data);
            if (m_logger_) m_logger_->debug("Wrote {} bytes to {} via backend", data.size(), relPath);
        } catch (...) {
            if (m_logger_) m_logger_->error("Failed to write bytes to {} via backend", relPath);
        }
    } else {
        if (m_logger_) m_logger_->debug("Received {} bytes for {} but no backend configured", data.size(), relPath);
    }
}

void SpdlogDebugger::flush() {
    if (m_backend_) {
        try { m_backend_->flush(); } catch (...) {}
    }
    try { spdlog::drop_all(); } catch (...) {}
}
