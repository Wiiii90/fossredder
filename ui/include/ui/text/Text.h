#pragma once

#include <QString>

namespace ui::text {

namespace status {

inline constexpr auto kReady = "Ready";
inline constexpr auto kImportCanceled = "Import canceled";
inline constexpr auto kImportFinished = "Import finished";
inline constexpr auto kImportFailed = "Import failed";

}

namespace actions {

inline constexpr auto kNewFile = "New...";
inline constexpr auto kOpenFile = "Open...";
inline constexpr auto kSaveFile = "Save";
inline constexpr auto kSaveFileAs = "Save As...";
inline constexpr auto kQuit = "Quit";
inline constexpr auto kAbout = "About";

}

namespace dialogs {

inline constexpr auto kSelectPdfTitle = "Select PDF";
inline constexpr auto kExportFileTitle = "Export File";
inline constexpr auto kNewFileTitle = "New File";
inline constexpr auto kOpenFileTitle = "Open File";
inline constexpr auto kSaveFileAsTitle = "Save File As";
inline constexpr auto kDatabaseFilter = "Database (*.db)";
inline constexpr auto kImportPdfFilter = "PDF Files (*.pdf)";
inline constexpr auto kExportFileFilter = "Excel Files (*.xlsx);;CSV Files (*.csv)";

}

namespace mainWindow {

inline constexpr auto kSelectedStatusPattern = "Selected: %1";
inline constexpr auto kExportPathStatusPattern = "Export path: %1";
inline constexpr auto kAboutTitle = "About FOSSRedder";
inline constexpr auto kAboutBody = "FOSSRedder";

}

namespace controllerErrors {

inline constexpr auto kStorageCreateFailed = "Failed to create file";
inline constexpr auto kStorageOpenFailed = "Failed to open file";
inline constexpr auto kStorageSaveFailed = "Failed to save file";
inline constexpr auto kStorageSaveAsFailed = "Failed to save file as";
inline constexpr auto kExportFailed = "Export failed";
inline constexpr auto kExportStateUnavailable = "Export state not available";
inline constexpr auto kImportControllerUnavailable = "Import controller not available";
inline constexpr auto kNoFileSelected = "No file selected";
inline constexpr auto kImportFailed = "Import failed";
inline constexpr auto kAnalysisEngineUnavailable = "Analysis engine not available";
inline constexpr auto kAnalysisStateUnavailable = "Analysis state not available";

}

namespace importRuns {

inline constexpr auto kTypeStatement = "Statement";
inline constexpr auto kStatusCanceled = "Canceled";
inline constexpr auto kStatusFailed = "Failed";
inline constexpr auto kStatusSuccess = "Success";

}

namespace importPhases {

inline constexpr auto kStopping = "Stopping...";
inline constexpr auto kStarting = "Starting import...";
inline constexpr auto kCanceled = "Import canceled";
inline constexpr auto kFailed = "Import failed";
inline constexpr auto kFinished = "Import finished";

}

namespace analysis {

inline constexpr auto kUnassignedContractType = "unassigned";

}

namespace exportRunner {

inline constexpr auto kRunnerUnavailable = "Export runner is not configured";
inline constexpr auto kStateSnapshotUnavailable = "Export state snapshot is null";

}

namespace contexts {

inline constexpr auto kImportState = "ImportState";
inline constexpr auto kAnalysisPayloadMapper = "AnalysisPayloadMapper";

}

namespace language {

inline constexpr auto kEnglishLabel = "English";
inline constexpr auto kGermanLabel = "Deutsch";

}

}
