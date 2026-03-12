/**
 * @file debug/include/debug/ErrorReporter.h
 * @brief Declares debugger-backed adapters for the core error reporting contracts.
 */

#pragma once

#include <memory>

#include "core/errors/IErrorReporter.h"
#include "debug/IDebugger.h"

namespace debug {

class DebuggerErrorReporter : public core::errors::IErrorReporter {
public:
    explicit DebuggerErrorReporter(std::shared_ptr<IDebugger> debugger);

    void report(const core::errors::ErrorEvent& event) override;

private:
    std::shared_ptr<IDebugger> debugger_;
};

std::shared_ptr<core::errors::IErrorReporter> createDefaultErrorReporter();

}
