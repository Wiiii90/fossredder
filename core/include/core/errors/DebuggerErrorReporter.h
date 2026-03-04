#pragma once

#include <memory>

#include "core/errors/IErrorReporter.h"
#include "debug/IDebugger.h"

namespace core::errors {

class DebuggerErrorReporter : public IErrorReporter {
public:
    explicit DebuggerErrorReporter(std::shared_ptr<IDebugger> debugger);

    void report(const ErrorEvent& event) override;
    void reportException(ErrorSeverity severity, const char* origin, std::exception_ptr exception) override;

private:
    std::shared_ptr<IDebugger> debugger_;
};

}