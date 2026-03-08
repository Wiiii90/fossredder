#pragma once

#include <QString>

namespace ui::controllers::contracts {

namespace operations {

inline const auto kNewFile = QStringLiteral("newFile");
inline const auto kOpenFile = QStringLiteral("openFile");
inline const auto kSaveFile = QStringLiteral("saveFile");
inline const auto kSaveFileAs = QStringLiteral("saveFileAs");

}

enum class ExportFormat : int {
    Csv = 0,
    Xlsx = 1
};

}
