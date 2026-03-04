#include "ui/controllers/StorageController.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

namespace ui {

namespace {

bool ensureCore(const AppStateController* core, const char* origin)
{
    if (core) return true;
    core::errors::report(core::errors::ErrorSeverity::Warning,
                         core::errors::codes::GenericError,
                         origin,
                         "AppStateController is null");
    return false;
}

void reportStorageException(const char* origin)
{
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::ExceptionError,
                                  origin,
                                  std::current_exception());
}

}

StorageController::StorageController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString StorageController::currentPath() const
{
    if (!ensureCore(core_, "ui::StorageController::currentPath")) return {};
    try {
        return QString::fromStdString(core_->currentPath());
    } catch (...) {
        reportStorageException("ui::StorageController::currentPath");
    }
    return {};
}

void StorageController::newFile(const QString& path)
{
    if (!ensureCore(core_, "ui::StorageController::newFile")) return;
    try {
        core_->newFile(path.toStdString());
        emit currentPathChanged();
    } catch (...) {
        reportStorageException("ui::StorageController::newFile");
    }
}

void StorageController::openFile(const QString& path)
{
    if (!ensureCore(core_, "ui::StorageController::openFile")) return;
    try {
        core_->openFile(path.toStdString());
        emit currentPathChanged();
    } catch (...) {
        reportStorageException("ui::StorageController::openFile");
    }
}

void StorageController::saveFile()
{
    if (!ensureCore(core_, "ui::StorageController::saveFile")) return;
    try {
        core_->saveFile();
        emit currentPathChanged();
    } catch (...) {
        reportStorageException("ui::StorageController::saveFile");
    }
}

void StorageController::saveFileAs(const QString& path)
{
    if (!ensureCore(core_, "ui::StorageController::saveFileAs")) return;
    try {
        core_->saveFileAs(path.toStdString());
        emit currentPathChanged();
    } catch (...) {
        reportStorageException("ui::StorageController::saveFileAs");
    }
}

}
