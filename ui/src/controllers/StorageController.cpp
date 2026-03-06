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
    return controllers::guard::invokeValue<QString>(core_, "ui::StorageController::currentPath", {}, [&]() {
        return QString::fromStdString(core_->currentPath());
    });
}

void StorageController::newFile(const QString& path)
{
    const bool success = controllers::guard::invokeValue<bool>(core_, "ui::StorageController::newFile", false, [&]() {
        core_->newFile(path.toStdString());
        return true;
    });

    if (!success) {
        setLastError(controllers::contracts::errors::kStorageCreateFailed);
        emit operationFailed(controllers::contracts::operations::kNewFile, lastError_);
        return;
    }

    setLastError({});
    emit currentPathChanged();
    emit operationSucceeded(controllers::contracts::operations::kNewFile);
}

void StorageController::openFile(const QString& path)
{
    const bool success = controllers::guard::invokeValue<bool>(core_, "ui::StorageController::openFile", false, [&]() {
        core_->openFile(path.toStdString());
        return true;
    });

    if (!success) {
        setLastError(controllers::contracts::errors::kStorageOpenFailed);
        emit operationFailed(controllers::contracts::operations::kOpenFile, lastError_);
        return;
    }

    setLastError({});
    emit currentPathChanged();
    emit operationSucceeded(controllers::contracts::operations::kOpenFile);
}

void StorageController::saveFile()
{
    const bool success = controllers::guard::invokeValue<bool>(core_, "ui::StorageController::saveFile", false, [&]() {
        core_->saveFile();
        return true;
    });

    if (!success) {
        setLastError(controllers::contracts::errors::kStorageSaveFailed);
        emit operationFailed(controllers::contracts::operations::kSaveFile, lastError_);
        return;
    }

    setLastError({});
    emit currentPathChanged();
    emit operationSucceeded(controllers::contracts::operations::kSaveFile);
}

void StorageController::saveFileAs(const QString& path)
{
    const bool success = controllers::guard::invokeValue<bool>(core_, "ui::StorageController::saveFileAs", false, [&]() {
        core_->saveFileAs(path.toStdString());
        return true;
    });

    if (!success) {
        setLastError(controllers::contracts::errors::kStorageSaveAsFailed);
        emit operationFailed(controllers::contracts::operations::kSaveFileAs, lastError_);
        return;
    }

    setLastError({});
    emit currentPathChanged();
    emit operationSucceeded(controllers::contracts::operations::kSaveFileAs);
}

}
