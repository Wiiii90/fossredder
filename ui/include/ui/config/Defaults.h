/**
 * @file ui/include/ui/config/Defaults.h
 * @brief Defines shared UI runtime constants, resource names and settings keys.
 */

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
inline const auto kTranslationResourcePrefix = QStringLiteral(":/i18n/");
inline const auto kTranslationBaseName = QStringLiteral("fossredder");
inline const auto kSettingsOrganizationName = QStringLiteral("FOSSRedder");
inline const auto kSettingsApplicationName = QStringLiteral("FOSSRedder");
inline constexpr auto kLanguageEnglishCode = "en";
inline constexpr auto kLanguageGermanCode = "de";
inline constexpr auto kLanguageSettingsKey = "general/language";
inline const auto kQrcQmlImportPath = QStringLiteral("qrc:/qml");

namespace qmlRegistration {

inline constexpr auto kContractsTypeName = "UIContracts";
inline constexpr auto kContractsTypeDescription = "UI controller contracts are exposed as enums only";

}

namespace errorCodes {

inline constexpr auto kExportRunnerUnavailable = "UI_EXPORT_RUNNER_UNAVAILABLE";

}

constexpr int kImportRunKeepCount = 20;

namespace importProgress {

constexpr double kMinimum = 0.0;
constexpr double kInitial = 0.01;
constexpr double kMaximum = 1.0;

}

namespace importPaging {

constexpr int kNone = 0;

}

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
