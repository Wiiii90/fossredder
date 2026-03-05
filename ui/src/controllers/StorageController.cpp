#include "ui/controllers/StorageController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/UiControllerContracts.h"

namespace ui {

void StorageController::setLastError(const QString& error)
{
    if (lastError_ == error) return;
    lastError_ = error;
    emit errorChanged();
}

StorageController::StorageController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString StorageController::currentPath() const
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::currentPath")) return {};
    try {
        return QString::fromStdString(core_->currentPath());
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::currentPath");
    }
    return {};
}

void StorageController::newFile(const QString& path)
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::newFile")) return;
    try {
        core_->newFile(path.toStdString());
        setLastError({});
        emit currentPathChanged();
        emit operationSucceeded(controllers::contracts::operations::kNewFile);
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::newFile");
        setLastError(controllers::contracts::errors::kStorageCreateFailed);
        emit operationFailed(controllers::contracts::operations::kNewFile, lastError_);
    }
}

void StorageController::openFile(const QString& path)
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::openFile")) return;
    try {
        core_->openFile(path.toStdString());
        setLastError({});
        emit currentPathChanged();
        emit operationSucceeded(controllers::contracts::operations::kOpenFile);
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::openFile");
        setLastError(controllers::contracts::errors::kStorageOpenFailed);
        emit operationFailed(controllers::contracts::operations::kOpenFile, lastError_);
    }
}

void StorageController::saveFile()
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::saveFile")) return;
    try {
        core_->saveFile();
        setLastError({});
        emit currentPathChanged();
        emit operationSucceeded(controllers::contracts::operations::kSaveFile);
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::saveFile");
        setLastError(controllers::contracts::errors::kStorageSaveFailed);
        emit operationFailed(controllers::contracts::operations::kSaveFile, lastError_);
    }
}

void StorageController::saveFileAs(const QString& path)
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::saveFileAs")) return;
    try {
        core_->saveFileAs(path.toStdString());
        setLastError({});
        emit currentPathChanged();
        emit operationSucceeded(controllers::contracts::operations::kSaveFileAs);
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::saveFileAs");
        setLastError(controllers::contracts::errors::kStorageSaveAsFailed);
        emit operationFailed(controllers::contracts::operations::kSaveFileAs, lastError_);
    }
}

}
