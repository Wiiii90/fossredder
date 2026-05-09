/**
 * @file ui/include/ui/util/CoreFacadeGuard.h
 * @brief Declares guard helpers for UI calls into the core application facade.
 */

#pragma once

#include <exception>
#include <functional>
#include <utility>

#include "core/application/workspace/WorkspaceFacade.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

namespace ui::util::guard {

/** @brief Ensure that a required application facade pointer is available. */
inline bool ensureCore(const core::application::WorkspaceFacade* core, const char* origin)
{
    if (core) {
        return true;
    }

    core::errors::report(core::errors::ErrorSeverity::Warning,
                         core::errors::codes::GenericError,
                         origin,
                         "WorkspaceFacade is null");
    return false;
}

/** @brief Report the current exception using the shared UI error reporting path. */
inline void reportException(const char* origin,
                            const char* code = core::errors::codes::ExceptionError)
{
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  code,
                                  origin,
                                  std::current_exception());
}

/** @brief Execute a void UI-to-core call with null-guarding and exception reporting. */
template <typename CorePtr, typename Func>
inline void invokeVoid(CorePtr core, const char* origin, Func&& func)
{
    if (!ensureCore(core, origin)) {
        return;
    }

    try {
        std::invoke(std::forward<Func>(func));
    } catch (const std::exception&) {
        reportException(origin, core::errors::codes::ExceptionStd);
    } catch (...) {
        reportException(origin, core::errors::codes::ExceptionNonStd);
    }
}

/** @brief Execute a value-returning UI-to-core call with fallback on failure. */
template <typename TValue, typename CorePtr, typename Func>
inline TValue invokeValue(CorePtr core, const char* origin, TValue fallback, Func&& func)
{
    if (!ensureCore(core, origin)) {
        return fallback;
    }

    try {
        return std::invoke(std::forward<Func>(func));
    } catch (const std::exception&) {
        reportException(origin, core::errors::codes::ExceptionStd);
    } catch (...) {
        reportException(origin, core::errors::codes::ExceptionNonStd);
    }
    return fallback;
}

} // namespace ui::util::guard
