/**
 * @file ui/src/controllers/StorageController.cpp
 * @brief Implements UI-facing storage operations and error propagation.
 */

#include "ui/controllers/StorageController.h"

#include "ui/config/Defaults.h"
#include "ui/observability/Origins.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"
#include "ui/text/Text.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/presenters/IWorkspacePresenter.h"

namespace ui {

bool StorageController::runCoreOperation(const char* context,
                                         const std::function<void()>& action)
{
    return ui::util::guard::invokeValue<bool>(core_, context, false, [&]() {
        action();
        return true;
    });
}

void StorageController::finishOperation(bool success,
                                        const QString& failureText,
                                        const QString& operation)
{
    if (!success) {
        emit operationFailed(operation, failureText);
        return;
    }

    emit operationSucceeded(operation);
    emit currentPathChanged();
}

StorageController::StorageController(core::ports::workspace::IWorkspaceWriter* core,
                                     std::shared_ptr<core::ports::presenters::IWorkspacePresenter> workspacePresenter,
                                     QObject* parent)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
    , workspacePresenter_(std::move(workspacePresenter))
{
}

void StorageController::newFile(const QString& path)
{
    const bool success =
        runCoreOperation(observability::origins::controller::storage::kNewFile,
                         [&]() { core_->newFile(strings::toEncodedPath(path)); });
    finishOperation(success, ui::text::controllerErrors::storageCreateFailed(),
                  ui::config::operationKeys::kNewFile);
}

void StorageController::openFile(const QString& path)
{
    const bool success = runCoreOperation(
        observability::origins::controller::storage::kOpenFile,
        [&]() { core_->openFile(strings::toEncodedPath(path)); });
    finishOperation(success, ui::text::controllerErrors::storageOpenFailed(),
                  ui::config::operationKeys::kOpenFile);
}

void StorageController::saveFile()
{
    const bool success =
        runCoreOperation(observability::origins::controller::storage::kSaveFile,
                         [&]() { core_->saveFile(); });
    finishOperation(success, ui::text::controllerErrors::storageSaveFailed(),
                  ui::config::operationKeys::kSaveFile);
}

void StorageController::saveFileAs(const QString& path)
{
    const bool success = runCoreOperation(
        observability::origins::controller::storage::kSaveFileAs,
        [&]() { core_->saveFileAs(strings::toEncodedPath(path)); });
    finishOperation(success, ui::text::controllerErrors::storageSaveAsFailed(),
                  ui::config::operationKeys::kSaveFileAs);
}

QString StorageController::currentPath() const
{
    if (!reader_) {
        return {};
    }

    const auto snapshot = reader_->workspaceSnapshot();
    core::ports::presenters::WorkspacePresentation presentation{snapshot.currentPath, snapshot.hasCurrentPath};
    const auto workspace = workspacePresenter_ ? workspacePresenter_->present(presentation)
                                               : presentation;
    return QString::fromStdString(workspace.currentPath);
}

} // namespace ui
