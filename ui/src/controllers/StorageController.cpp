#include "ui/controllers/StorageController.h"

#include "ui/controllers/ControllerGuard.h"

namespace ui {

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
        emit currentPathChanged();
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::newFile");
    }
}

void StorageController::openFile(const QString& path)
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::openFile")) return;
    try {
        core_->openFile(path.toStdString());
        emit currentPathChanged();
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::openFile");
    }
}

void StorageController::saveFile()
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::saveFile")) return;
    try {
        core_->saveFile();
        emit currentPathChanged();
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::saveFile");
    }
}

void StorageController::saveFileAs(const QString& path)
{
    if (!controllers::guard::ensureCore(core_, "ui::StorageController::saveFileAs")) return;
    try {
        core_->saveFileAs(path.toStdString());
        emit currentPathChanged();
    } catch (...) {
        controllers::guard::reportException("ui::StorageController::saveFileAs");
    }
}

}
