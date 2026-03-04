#include "core/errors/ErrorReporterRegistry.h"

#include <mutex>

namespace core::errors {

namespace {

std::shared_ptr<IErrorReporter> g_reporter;
std::mutex g_mutex;

}

void setGlobalErrorReporter(std::shared_ptr<IErrorReporter> reporter)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_reporter = std::move(reporter);
}

std::shared_ptr<IErrorReporter> globalErrorReporter()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_reporter;
}

void report(const ErrorEvent& event)
{
    auto reporter = globalErrorReporter();
    if (reporter) reporter->report(event);
}

void reportException(ErrorSeverity severity, const char* origin, std::exception_ptr exception)
{
    auto reporter = globalErrorReporter();
    if (reporter) reporter->reportException(severity, origin, exception);
}

}