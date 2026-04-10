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
inline const auto kFileSystemController = QStringLiteral("fileSystemController");
inline const auto kStatus = QStringLiteral("status");
inline const auto kStorageController = QStringLiteral("storageController");
inline const auto kAnnualController = QStringLiteral("annualController");
inline const auto kActorController = QStringLiteral("actorController");
inline const auto kPropertyController = QStringLiteral("propertyController");
inline const auto kContractController = QStringLiteral("contractController");
inline const auto kStatementController = QStringLiteral("statementController");
inline const auto kTransactionController = QStringLiteral("transactionController");
inline const auto kDraftController = QStringLiteral("draftController");
inline const auto kAnalysisController = QStringLiteral("analysisController");
inline const auto kExportController = QStringLiteral("exportController");
inline const auto kImportController = QStringLiteral("importController");
inline const auto kLanguageController = QStringLiteral("languageController");
inline const auto kIsDebugBuild = QStringLiteral("isDebugBuild");
}

/** Names of any QML image providers exposed by the application. */
namespace providers {
inline const auto kImportProof = QStringLiteral("importProof");
}

/** Convenience property keys used by QML components. */
namespace properties {
inline constexpr auto kWidth = "width";
inline constexpr auto kHeight = "height";
}

/** Supported export formats exposed to QML. */
enum class ExportFormat : int {
    Csv = 0,
    Xlsx = 1
};

Q_ENUM_NS(ExportFormat)

} // namespace ui::qml::contracts

namespace ui::qml {

struct QmlContractsRegistration {
    Q_GADGET
    QML_FOREIGN_NAMESPACE(ui::qml::contracts)
    QML_NAMED_ELEMENT(QmlContracts)
};

} // namespace ui::qml
