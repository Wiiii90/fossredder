#include "core/errors/ErrorReporterRegistry.h"
#include "core/errors/ErrorCodes.h"

#include <mutex>
#include <typeinfo>

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

void report(ErrorSeverity severity, const char* code, const char* origin, std::string message, ErrorContext context)
{
    ErrorEvent event;
    event.severity = severity;
    event.code = code ? code : codes::GenericError;
    event.origin = origin ? origin : std::string{};
    event.message = std::move(message);
    event.context = std::move(context);
    report(event);
}

void reportException(ErrorSeverity severity, const char* origin, std::exception_ptr exception)
{
    reportException(severity, codes::ExceptionError, origin, exception, {});
}

void reportException(ErrorSeverity severity, const char* code, const char* origin, std::exception_ptr exception, ErrorContext context)
{
    auto reporter = globalErrorReporter();
    if (!reporter) return;

    if (!exception) {
        ErrorEvent event;
        event.severity = severity;
        event.code = code ? code : codes::ExceptionError;
        event.origin = origin ? origin : std::string{};
        event.message = "unknown exception";
        event.exceptionType = "unknown";
        event.context = std::move(context);
        reporter->report(event);
        return;
    }

    try {
        std::rethrow_exception(exception);
    } catch (const std::exception& ex) {
        ErrorEvent event;
        event.severity = severity;
        event.code = code ? code : codes::ExceptionStd;
        event.origin = origin ? origin : std::string{};
        event.message = ex.what();
        event.exceptionType = typeid(ex).name();
        event.context = std::move(context);
        reporter->report(event);
    } catch (...) {
        ErrorEvent event;
        event.severity = severity;
        event.code = code ? code : codes::ExceptionNonStd;
        event.origin = origin ? origin : std::string{};
        event.message = "non-std exception";
        event.exceptionType = "unknown";
        event.context = std::move(context);
        reporter->report(event);
    }
}

}