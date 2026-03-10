#pragma once

namespace core::errors::codes {

inline constexpr const char* GenericError = "GENERIC_ERROR";

inline constexpr const char* ExceptionError = "EXCEPTION_ERROR";
inline constexpr const char* ExceptionStd = "EXCEPTION_STD";
inline constexpr const char* ExceptionNonStd = "EXCEPTION_NON_STD";

inline constexpr const char* QtDebug = "QT_DEBUG";
inline constexpr const char* QtInfo = "QT_INFO";
inline constexpr const char* QtWarning = "QT_WARNING";
inline constexpr const char* QtCritical = "QT_CRITICAL";
inline constexpr const char* QtFatal = "QT_FATAL";

inline constexpr const char* QmlWarning = "QML_WARNING";

inline constexpr const char* UiFlowImportStarted = "UI_FLOW_IMPORT_STARTED";
inline constexpr const char* UiFlowImportFinished = "UI_FLOW_IMPORT_FINISHED";
inline constexpr const char* UiFlowImportCanceled = "UI_FLOW_IMPORT_CANCELED";
inline constexpr const char* UiFlowImportFailed = "UI_FLOW_IMPORT_FAILED";
inline constexpr const char* UiFlowImportRejected = "UI_FLOW_IMPORT_REJECTED";
inline constexpr const char* UiFlowExportStarted = "UI_FLOW_EXPORT_STARTED";
inline constexpr const char* UiFlowExportFinished = "UI_FLOW_EXPORT_FINISHED";
inline constexpr const char* UiFlowExportFailed = "UI_FLOW_EXPORT_FAILED";
inline constexpr const char* UiFlowExportFallback = "UI_FLOW_EXPORT_FALLBACK";
inline constexpr const char* UiFlowMainWindowAction = "UI_FLOW_MAINWINDOW_ACTION";

inline constexpr const char* ConfigDbOpenFailed = "CFG_DB_OPEN_FAILED";

}