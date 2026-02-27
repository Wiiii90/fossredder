#include "ui/controllers/StorageController.h"

namespace ui {

StorageController::StorageController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString StorageController::currentPath() const
{
    if (!core_) return {};
    return QString::fromStdString(core_->currentPath());
}

void StorageController::newFile(const QString& path)
{
    if (!core_) return;
    core_->newFile(path.toStdString());
    emit currentPathChanged();
}

void StorageController::openFile(const QString& path)
{
    if (!core_) return;
    core_->openFile(path.toStdString());
    emit currentPathChanged();
}

void StorageController::saveFile()
{
    if (!core_) return;
    core_->saveFile();
    emit currentPathChanged();
}

void StorageController::saveFileAs(const QString& path)
{
    if (!core_) return;
    core_->saveFileAs(path.toStdString());
    emit currentPathChanged();
}

}
