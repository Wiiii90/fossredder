#pragma once

#include <QString>

namespace ui::config {

inline const auto kMainWindowTitle = QStringLiteral("FOSSRedder");
inline const auto kMainQmlSource = QStringLiteral("qrc:/qml/FossRedder/Main.qml");

constexpr int kMainWindowDefaultWidth = 1200;
constexpr int kMainWindowDefaultHeight = 800;

inline const auto kAppQmlDirName = QStringLiteral("qml");
inline const auto kImageFormatsDirName = QStringLiteral("imageformats");
inline const auto kTranslationsDirName = QStringLiteral("i18n");
inline const auto kTranslationBaseName = QStringLiteral("fossredder");
inline constexpr auto kLanguageEnglishCode = "en";
inline constexpr auto kLanguageGermanCode = "de";
inline constexpr auto kLanguageSettingsKey = "general/language";
inline const auto kQrcQmlImportPath = QStringLiteral("qrc:/qml");

constexpr int kImportRunKeepCount = 20;
constexpr double kInitialImportProgress = 0.01;

inline const auto kImportProgressPagePattern = QStringLiteral("\\[(\\d+)\\s*/\\s*(\\d+)\\]");
inline const auto kImportRunTimestampFormat = QStringLiteral("yyyyMMddHHmmsszzz");
inline const auto kImportRunNamePattern = QStringLiteral("%1_import_%2");
inline const auto kImportRunGlobPattern = QStringLiteral("*_import_*");
constexpr int kImportRunFirstSuffix = 1;

inline const auto kJsonEmptyObject = QStringLiteral("{}");

namespace objectNames {

inline constexpr auto kActionNewFile = "actionNewFile";
inline constexpr auto kActionOpenFile = "actionOpenFile";
inline constexpr auto kActionSaveFile = "actionSaveFile";
inline constexpr auto kActionSaveFileAs = "actionSaveFileAs";
inline constexpr auto kActionQuit = "actionQuit";
inline constexpr auto kActionAbout = "actionAbout";

}

}
