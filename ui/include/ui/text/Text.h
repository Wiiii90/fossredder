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

namespace analysis {

inline const auto kUnassignedContractType = QStringLiteral("unassigned");

}

}
