#pragma once

#include <QObject>
#include <QString>

namespace ui::controllers::contracts {

Q_NAMESPACE

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

Q_ENUM_NS(ExportFormat)

}
