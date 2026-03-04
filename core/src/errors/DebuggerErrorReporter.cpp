#include "core/errors/DebuggerErrorReporter.h"

#include <sstream>
#include <typeinfo>

namespace core::errors {

namespace {

const char* toString(ErrorSeverity severity)
{
    switch (severity) {
    case ErrorSeverity::Info: return "info";
    case ErrorSeverity::Warning: return "warning";
    case ErrorSeverity::Error: return "error";
    case ErrorSeverity::Critical: return "critical";
    }
    return "error";
}

}

DebuggerErrorReporter::DebuggerErrorReporter(std::shared_ptr<IDebugger> debugger)
    : debugger_(std::move(debugger))
{
}

void DebuggerErrorReporter::report(const ErrorEvent& event)
{
    if (!debugger_ || !debugger_->enabled()) return;

    std::ostringstream out;
    out << "[" << toString(event.severity) << "] ";
    if (!event.origin.empty()) out << event.origin << " - ";
    out << event.message;
    if (!event.exceptionType.empty()) out << " (" << event.exceptionType << ")";
    out << "\n";
    debugger_->writeText("errors/errors.log", out.str());
}

void DebuggerErrorReporter::reportException(ErrorSeverity severity, const char* origin, std::exception_ptr exception)
{
    if (!exception) {
        report({severity, origin ? origin : std::string{}, "unknown exception", "unknown"});
        return;
    }

    try {
        std::rethrow_exception(exception);
    } catch (const std::exception& ex) {
        report({severity, origin ? origin : std::string{}, ex.what(), typeid(ex).name()});
    } catch (...) {
        report({severity, origin ? origin : std::string{}, "non-std exception", "unknown"});
    }
}

}