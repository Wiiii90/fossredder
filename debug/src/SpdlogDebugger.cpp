/**
 * @file debug/src/SpdlogDebugger.cpp
 * @brief Implements a debugger backend that mirrors events to spdlog sinks.
 */

#include "debug/pch.h"
#include "debug/DebugDefaults.h"
#include "debug/SpdlogDebugger.h"

#include <filesystem>

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
        const std::filesystem::path logPath(debug::defaults::kSpdlogFilePath);
        if (logPath.has_parent_path()) std::filesystem::create_directories(logPath.parent_path());
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logPath.string(),
            debug::defaults::kSpdlogFileSizeBytes,
            debug::defaults::kSpdlogFileCount);
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
    } catch (...) {
        m_enabled_ = false;
    }
}

bool SpdlogDebugger::enabled() const {
    return m_enabled_;
}

void SpdlogDebugger::writeText(const std::string& relPath, const std::string& text) {
    if (!m_enabled_) return;

    try {
        if (relPath.rfind(std::string(debug::defaults::kPopplerMetadataPrefix), 0) == 0) {
            if (m_backend_) {
                try { m_backend_->writeText(relPath, text); } catch (...) { if (m_logger_) m_logger_->warn("backend writeText failed for {}", relPath); }
            }
            if (m_logger_) m_logger_->debug("[{}] metadata written (content omitted)", relPath);
            return;
        }
    } catch (...) {
        if (m_logger_) m_logger_->warn("writeText pre-check failed for {}", relPath);
    }

    if (m_logger_) m_logger_->info("[{}] {}", relPath, text);
    if (m_backend_) {
        try { m_backend_->writeText(relPath, text); } catch (...) { if (m_logger_) m_logger_->warn("backend writeText failed for {}", relPath); }
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
        try { m_backend_->flush(); } catch (...) { if (m_logger_) m_logger_->warn("backend flush failed"); }
    }
    try { spdlog::drop_all(); } catch (...) { if (m_logger_) m_logger_->warn("spdlog drop_all failed"); }
}
