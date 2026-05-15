/**
 * @file ui/include/ui/observability/Origins.h
 * @brief Declarations for the UI Origins component.
 */

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

namespace workflow {
namespace analysis {
inline constexpr auto kAdd = "ui::AnalysisWorkflow::addAnalysis";
inline constexpr auto kUpdate = "ui::AnalysisWorkflow::updateAnalysis";
inline constexpr auto kDelete = "ui::AnalysisWorkflow::deleteAnalysis";
inline constexpr auto kCompute = "ui::AnalysisWorkflow::computeAnalysis";
}
namespace exportFlow {
inline constexpr auto kStart = "ui::ExportWorkflow::exportData";
inline constexpr auto kFinish = "ui::ExportWorkflow::onExportFinished";
}
}

namespace workspace {
namespace annual {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addAnnual";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateAnnual";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteAnnual";
}
namespace actor {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addActor";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateActor";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteActor";
}
namespace contract {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addContract";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateContract";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteContract";
}
namespace property {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addProperty";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateProperty";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteProperty";
}
namespace storage {
inline constexpr auto kNewFile = "ui::WorkspaceFacade::newFile";
inline constexpr auto kOpenFile = "ui::WorkspaceFacade::openFile";
inline constexpr auto kSaveFile = "ui::WorkspaceFacade::saveFile";
inline constexpr auto kSaveFileAs = "ui::WorkspaceFacade::saveFileAs";
}
namespace statement {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addStatement";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateStatement";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteStatement";
}
namespace transaction {
inline constexpr auto kAdd = "ui::WorkspaceFacade::addTransaction";
inline constexpr auto kUpdate = "ui::WorkspaceFacade::updateTransaction";
inline constexpr auto kDelete = "ui::WorkspaceFacade::deleteTransaction";
}
}

namespace workflow {
namespace import {
inline constexpr auto kFinalize = "ui::ImportWorkflow::finalizeStatementDraft";
inline constexpr auto kStart = "ui::ImportWorkflow::startStatementImport";
inline constexpr auto kCancel = "ui::ImportWorkflow::cancelImport";
inline constexpr auto kCancelAll = "ui::ImportWorkflow::cancelAllImports";
inline constexpr auto kTerminal = "ui::ImportWorkflow::onJobTerminal";
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
