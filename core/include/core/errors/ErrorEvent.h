#pragma once

#include <string>

namespace core::errors {

enum class ErrorSeverity {
    Info,
    Warning,
    Error,
    Critical
};

struct ErrorEvent {
    ErrorSeverity severity = ErrorSeverity::Error;
    std::string origin;
    std::string message;
    std::string exceptionType;
};

}