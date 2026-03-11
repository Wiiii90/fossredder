#pragma once

#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <exception>
#include <string_view>

namespace debug {
std::string startRun(const std::string& processName);
void endRun();
}

namespace core::errors {

class ScopedDebugRun {
public:
    explicit ScopedDebugRun(std::string_view runName,
                            const char* origin = "core::errors::ScopedDebugRun")
        : origin_(origin)
    {
        try {
            debug::startRun(std::string(runName));
            active_ = true;
        } catch (...) {
            reportException(ErrorSeverity::Warning, origin_, std::current_exception());
        }
    }

    ScopedDebugRun(const ScopedDebugRun&) = delete;
    ScopedDebugRun& operator=(const ScopedDebugRun&) = delete;

    ScopedDebugRun(ScopedDebugRun&& other) noexcept
        : active_(other.active_)
        , origin_(other.origin_)
    {
        other.active_ = false;
    }

    ScopedDebugRun& operator=(ScopedDebugRun&& other) noexcept
    {
        if (this == &other) return *this;
        finish();
        active_ = other.active_;
        origin_ = other.origin_;
        other.active_ = false;
        return *this;
    }

    ~ScopedDebugRun()
    {
        finish();
    }

private:
    void finish() noexcept
    {
        if (!active_) return;
        try {
            debug::endRun();
        } catch (...) {
            reportException(ErrorSeverity::Warning, origin_, std::current_exception());
        }
        active_ = false;
    }

    bool active_ = false;
    const char* origin_ = nullptr;
};

}
