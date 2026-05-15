/**
 * @file ui/include/ui/bootstrap/QmlContracts.h
 * @brief QML registration and context property name constants used by the UI.
 */

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace ui::qml::contracts {

Q_NAMESPACE

/** Module/version and type registration metadata for the QML module. */
namespace module {
inline constexpr auto kName = "FossRedder";
inline constexpr auto kMainTypeName = "Main";
constexpr int kMajorVersion = 1;
constexpr int kMinorVersion = 0;
inline constexpr auto kAppContextTypeName = "AppContext";
inline constexpr auto kQmlContractsTypeName = "QmlContracts";
inline constexpr auto kQmlContractsTypeDescription = "QML contracts are exposed as enums only";
inline constexpr auto kNavigationTypeName = "Navigation";
inline constexpr auto kNavigationTypeDescription = "Navigation is exposed via context property 'navigation'";
}

/** Names of context properties set on the QQmlContext. */
namespace context {
inline const auto kActions = QStringLiteral("actions");
inline const auto kNavigation = QStringLiteral("navigation");
inline const auto kSession = QStringLiteral("session");
inline const auto kWorkspace = QStringLiteral("workspace");
inline const auto kWorkspaceFacade = QStringLiteral("workspaceFacade");
inline const auto kFileSystemBrowser = QStringLiteral("fileSystemBrowser");
inline const auto kStatus = QStringLiteral("status");
inline const auto kAnalysisWorkflow = QStringLiteral("analysisWorkflow");
inline const auto kExportWorkflow = QStringLiteral("exportWorkflow");
inline const auto kImportWorkflow = QStringLiteral("importWorkflow");
inline const auto kLanguageService = QStringLiteral("languageService");
inline const auto kSettingsViewModel = QStringLiteral("settingsViewModel");
inline const auto kIsDebugBuild = QStringLiteral("isDebugBuild");
}

/** Convenience property keys used by QML components. */
namespace properties {
inline constexpr auto kWidth = "width";
inline constexpr auto kHeight = "height";
}

/** Reusable QML labels and object names. */
namespace labels {
inline constexpr auto kAdd = "+";
inline constexpr auto kRemove = "-";
inline constexpr auto kCreate = "Create";
inline constexpr auto kClear = "Clear";
inline constexpr auto kDelete = "Delete";
inline constexpr auto kUpdate = "Update";
inline constexpr auto kActorCreateModeButton = "actorCreateModeButton";
inline constexpr auto kPropertyCreateModeButton = "propertyCreateModeButton";
inline constexpr auto kContractCreateModeButton = "contractCreateModeButton";
}

/** Supported export formats exposed to QML. */
enum class ExportFormat : int {
    Csv = 0,
    Xlsx = 1
};

Q_ENUM_NS(ExportFormat)

enum class NavigationValue : int {
    SectionActors = 0,
    SectionProperties = 1,
    SectionContracts = 2,
    SectionBooking = 3,
    SectionImport = 4,
    SectionExport = 5,
    SectionSettings = 6,
    SectionAnalysis = 7,
    SectionAnnual = 8,
    BookingStatements = 0,
    BookingCalendar = 1,
    BookingTransactions = 2,
    SettingsGeneral = 0,
    SettingsImport = 1,
    SettingsExport = 2,
    SettingsMiscellaneous = 3
};

Q_ENUM_NS(NavigationValue)

} // namespace ui::qml::contracts

namespace ui::qml {

struct QmlContractsRegistration {
    Q_GADGET
    QML_FOREIGN_NAMESPACE(ui::qml::contracts)
    QML_NAMED_ELEMENT(QmlContracts)
};

} // namespace ui::qml
