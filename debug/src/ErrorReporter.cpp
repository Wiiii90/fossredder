#include "debug/ErrorReporter.h"

#include "core/errors/ErrorReporting.h"
#include "debug/DebugDefaults.h"
#include "debug/FileDebugger.h"

#include <sstream>
#include <string>
#include <utility>

namespace debug {

DebuggerErrorReporter::DebuggerErrorReporter(std::shared_ptr<IDebugger> debugger)
    : debugger_(std::move(debugger))
{
}

void DebuggerErrorReporter::report(const core::errors::ErrorEvent& event)
{
    if (!debugger_ || !debugger_->enabled()) return;

    std::ostringstream out;
    out << "[" << core::errors::severityToString(event.severity) << "] ";
    out << "[" << (event.code.empty() ? core::errors::codes::GenericError : event.code) << "] ";
    if (!event.origin.empty()) out << event.origin << " - ";
    out << event.message;
    if (!event.context.empty()) {
        out << " {";
        bool first = true;
        for (const auto& kv : event.context) {
            if (!first) out << ", ";
            first = false;
            out << kv.first << "=" << kv.second;
        }
        out << "}";
    }
    if (!event.exceptionType.empty()) out << " (" << event.exceptionType << ")";
    out << "\n";
    debugger_->writeText(std::string(debug::defaults::kErrorLogPath), out.str());
}

std::shared_ptr<core::errors::IErrorReporter> createDefaultErrorReporter()
{
    return std::make_shared<DebuggerErrorReporter>(
        std::make_shared<FileDebugger>("", std::string(debug::defaults::kErrorsProcessName)));
}

}
