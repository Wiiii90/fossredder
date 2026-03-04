#pragma once

#include <memory>

#include "core/errors/IErrorReporter.h"

namespace core::errors {

void setGlobalErrorReporter(std::shared_ptr<IErrorReporter> reporter);
std::shared_ptr<IErrorReporter> globalErrorReporter();

void report(const ErrorEvent& event);
void reportException(ErrorSeverity severity, const char* origin, std::exception_ptr exception);

}