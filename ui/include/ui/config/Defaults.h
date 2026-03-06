#pragma once

#include <QString>

namespace ui::config {

inline const auto kMainWindowTitle = QStringLiteral("FOSSRedder");

constexpr int kMainWindowDefaultWidth = 1200;
constexpr int kMainWindowDefaultHeight = 800;

inline const auto kAppQmlDirName = QStringLiteral("qml");
inline const auto kImageFormatsDirName = QStringLiteral("imageformats");
inline const auto kQrcQmlImportPath = QStringLiteral("qrc:/qml");

inline const auto kImportProgressPagePattern = QStringLiteral("\\[(\\d+)\\s*/\\s*(\\d+)\\]");

inline const auto kJsonEmptyObject = QStringLiteral("{}");

}
