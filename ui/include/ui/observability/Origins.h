#pragma once

namespace ui::observability::origins {

namespace app {
inline constexpr auto kQmlWarnings = "app::qml::warnings";
inline constexpr auto kToCoreExportFormat = "app::toCoreExportFormat";
}

namespace mainWindow {
inline constexpr auto kActionRouting = "ui::MainWindow::setupActionRouting";
inline constexpr auto kLoadQml = "ui::MainWindow::loadQml";
inline constexpr auto kDragDrop = "ui::MainWindow::eventFilter";
inline constexpr auto kClose = "ui::MainWindow::closeEvent";
inline constexpr auto kCloseSucceeded = "ui::MainWindow::handleStorageOperationSucceeded";
inline constexpr auto kCloseFailed = "ui::MainWindow::handleStorageOperationFailed";
}

namespace controller {
namespace annual {
inline constexpr auto kAdd = "ui::AnnualController::addAnnual";
inline constexpr auto kUpdate = "ui::AnnualController::updateAnnual";
inline constexpr auto kDelete = "ui::AnnualController::deleteAnnual";
}
namespace actor {
inline constexpr auto kAdd = "ui::ActorController::addActor";
inline constexpr auto kUpdate = "ui::ActorController::updateActor";
inline constexpr auto kDelete = "ui::ActorController::deleteActor";
}
namespace analysis {
inline constexpr auto kAdd = "ui::AnalysisController::addAnalysis";
inline constexpr auto kCompute = "ui::AnalysisController::computeAnalysis";
}
namespace contract {
inline constexpr auto kAdd = "ui::ContractController::addContract";
inline constexpr auto kUpdate = "ui::ContractController::updateContract";
inline constexpr auto kDelete = "ui::ContractController::deleteContract";
inline constexpr auto kGetTypes = "ui::ContractController::getContractTypes";
}
namespace draft {
inline constexpr auto kFinalize = "ui::DraftController::finalizeStatementDraft";
}
namespace exportFlow {
inline constexpr auto kStart = "ui::ExportController::exportData";
inline constexpr auto kFinish = "ui::ExportController::onExportFinished";
}
namespace importFlow {
inline constexpr auto kStart = "ui::ImportController::startImportForFile";
inline constexpr auto kCancel = "ui::ImportController::cancelImport";
inline constexpr auto kCancelAll = "ui::ImportController::cancelAllImports";
inline constexpr auto kTerminal = "ui::ImportController::onJobTerminal";
}
namespace property {
inline constexpr auto kAdd = "ui::PropertyController::addProperty";
inline constexpr auto kUpdate = "ui::PropertyController::updateProperty";
inline constexpr auto kDelete = "ui::PropertyController::deleteProperty";
}
namespace storage {
inline constexpr auto kCurrentPath = "ui::StorageController::currentPath";
inline constexpr auto kNewFile = "ui::StorageController::newFile";
inline constexpr auto kOpenFile = "ui::StorageController::openFile";
inline constexpr auto kSaveFile = "ui::StorageController::saveFile";
inline constexpr auto kSaveFileAs = "ui::StorageController::saveFileAs";
}
namespace statement {
inline constexpr auto kAdd = "ui::StatementController::addStatement";
inline constexpr auto kUpdate = "ui::StatementController::updateStatement";
inline constexpr auto kDelete = "ui::StatementController::deleteStatement";
}
namespace transaction {
inline constexpr auto kAdd = "ui::TransactionController::addTransaction";
inline constexpr auto kUpdate = "ui::TransactionController::updateTransaction";
inline constexpr auto kDelete = "ui::TransactionController::deleteTransaction";
}
}

namespace model {
namespace analysisList {
inline constexpr auto kAdjustmentsJson = "ui::AnalysisList::data::adjustmentsJson";
inline constexpr auto kAdjustmentsParse = "ui::AnalysisList::setAdjustmentsById::parseJson";
inline constexpr auto kSetAdjustments = "ui::AnalysisList::setAdjustmentsById";
}
}

namespace service {
namespace exportRunner {
inline constexpr auto kRun = "ui::exporting::ExportRunner::run";
}
namespace importJobBridge {
inline constexpr auto kStartImport = "ui::importing::ImportJobBridge::startStatementImport";
inline constexpr auto kClearSubscription = "ui::importing::ImportJobBridge::clearSubscription";
}
}

}
