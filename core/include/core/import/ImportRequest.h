/**
 * @file core/include/core/import/ImportRequest.h
 * @brief Declares the import execution request contract.
 */

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace core { namespace jobs { class Scheduler; class SlotLimiter; } }

struct ImportRequest {
    std::string sourcePath;
    std::string runRoot;
    std::string runIdPrefix;
    std::string jobId;
    std::function<void(double, const std::string&)> progressCallback;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
    core::jobs::Scheduler* scheduler = nullptr;
    core::jobs::SlotLimiter* ocrLimiter = nullptr;
};
