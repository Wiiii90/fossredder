#pragma once

#include <QString>

namespace ui::controllers::contracts {

namespace operations {

inline const auto kNewFile = QStringLiteral("newFile");
inline const auto kOpenFile = QStringLiteral("openFile");
inline const auto kSaveFile = QStringLiteral("saveFile");
inline const auto kSaveFileAs = QStringLiteral("saveFileAs");

}

namespace errors {

inline const auto kStorageCreateFailed = QStringLiteral("Failed to create file");
inline const auto kStorageOpenFailed = QStringLiteral("Failed to open file");
inline const auto kStorageSaveFailed = QStringLiteral("Failed to save file");
inline const auto kStorageSaveAsFailed = QStringLiteral("Failed to save file as");
inline const auto kExportFailed = QStringLiteral("Export failed");
inline const auto kImportControllerUnavailable = QStringLiteral("Import controller not available");
inline const auto kNoFileSelected = QStringLiteral("No file selected");
inline const auto kImportFailed = QStringLiteral("Import failed");

}

namespace importRuns {

inline const auto kTypeStatement = QStringLiteral("Statement");
inline const auto kStatusCanceled = QStringLiteral("Canceled");
inline const auto kStatusFailed = QStringLiteral("Failed");
inline const auto kStatusSuccess = QStringLiteral("Success");

}

namespace importPhases {

inline const auto kStopping = QStringLiteral("Stopping...");
inline const auto kStarting = QStringLiteral("Starting import...");
inline const auto kCanceled = QStringLiteral("Import canceled");
inline const auto kFailed = QStringLiteral("Import failed");
inline const auto kFinished = QStringLiteral("Import finished");

}

enum class ExportFormat : int {
    Csv = 0,
    Xlsx = 1
};

}