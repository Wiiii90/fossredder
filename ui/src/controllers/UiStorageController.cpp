#include "ui/controllers/UiStorageController.h"

UiStorageController::UiStorageController(AppStateController* core, QObject* parent)
    : QObject(parent), core_(core) {

    if (core_) {
        core_->setStateChangedCallback([this](const AppState&) {
            emit stateLoaded();
        });
        core_->setDeletionImpactCallback([this](const DeletionImpact& impact) {
            emit deletionImpact(impact);
        });
    }
}

QString UiStorageController::currentPath() const {
    if (!core_) return {};
    return QString::fromStdString(core_->currentPath());
}

void UiStorageController::newFile(const QString& path) {
    if (!core_) return;
    core_->newFile(path.toStdString());
    emit currentPathChanged();
}

void UiStorageController::openFile(const QString& path) {
    if (!core_) return;
    core_->openFile(path.toStdString());
    emit currentPathChanged();
}

void UiStorageController::saveFile() {
    if (!core_) return;
    core_->saveFile();
    emit currentPathChanged();
}

void UiStorageController::saveFileAs(const QString& path) {
    if (!core_) return;
    core_->saveFileAs(path.toStdString());
    emit currentPathChanged();
}
