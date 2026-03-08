#pragma once

#include <QString>

namespace ui::qml::contracts {

namespace module {
inline constexpr auto kName = "FossRedder";
constexpr int kMajorVersion = 1;
constexpr int kMinorVersion = 0;
inline constexpr auto kNavigationTypeName = "UiNavigation";
inline constexpr auto kNavigationTypeDescription = "UiNavigation is exposed via context property 'uiNav'";
}

namespace context {
inline const auto kActions = QStringLiteral("uiActions");
inline const auto kNavigation = QStringLiteral("uiNav");
inline const auto kData = QStringLiteral("uiData");
inline const auto kFileSystemController = QStringLiteral("fileSystemController");
inline const auto kStatus = QStringLiteral("uiStatus");
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
inline const auto kIsDebugBuild = QStringLiteral("isDebugBuild");
}

namespace providers {
inline const auto kImportProof = QStringLiteral("importProof");
}

namespace properties {
inline constexpr auto kWidth = "width";
inline constexpr auto kHeight = "height";
}

}
