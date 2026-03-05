#include "ui/controllers/StorageController.h"

#include "ui/controllers/ControllerGuard.h"

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
        emit operationSucceeded(QStringLiteral("newFile"));
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::newFile");
        setLastError(QStringLiteral("Failed to create file"));
        emit operationFailed(QStringLiteral("newFile"), lastError_);
    }
}

void StorageController::openFile(const QString& path)
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::openFile")) return;
    try {
        core_->openFile(path.toStdString());
        setLastError({});
        emit currentPathChanged();
        emit operationSucceeded(QStringLiteral("openFile"));
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::openFile");
        setLastError(QStringLiteral("Failed to open file"));
        emit operationFailed(QStringLiteral("openFile"), lastError_);
    }
}

void StorageController::saveFile()
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::saveFile")) return;
    try {
        core_->saveFile();
        setLastError({});
        emit currentPathChanged();
        emit operationSucceeded(QStringLiteral("saveFile"));
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::saveFile");
        setLastError(QStringLiteral("Failed to save file"));
        emit operationFailed(QStringLiteral("saveFile"), lastError_);
    }
}

void StorageController::saveFileAs(const QString& path)
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::saveFileAs")) return;
    try {
        core_->saveFileAs(path.toStdString());
        setLastError({});
        emit currentPathChanged();
        emit operationSucceeded(QStringLiteral("saveFileAs"));
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::saveFileAs");
        setLastError(QStringLiteral("Failed to save file as"));
        emit operationFailed(QStringLiteral("saveFileAs"), lastError_);
    }
}

}
