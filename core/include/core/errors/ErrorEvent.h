#pragma once

#include <utility>
#include <string>
#include <vector>

namespace core::errors {

enum class ErrorSeverity {
    Info,
    Warning,
    Error,
    Critical
};

using ErrorContext = std::vector<std::pair<std::string, std::string>>;

struct ErrorEvent {
    ErrorSeverity severity = ErrorSeverity::Error;
    std::string origin;
    std::string message;
    std::string exceptionType;
    std::string code = "GENERIC_ERROR";
    ErrorContext context;
};

}