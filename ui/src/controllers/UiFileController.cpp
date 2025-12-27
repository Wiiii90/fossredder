#include "ui/controllers/UiFileController.h"

UiFileController::UiFileController(FileController* core, QObject* parent)
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

QString UiFileController::currentPath() const {
    if (!core_) return {};
    return QString::fromStdString(core_->currentPath());
}

void UiFileController::newFile(const QString& path) {
    if (!core_) return;
    core_->newFile(path.toStdString());
    emit currentPathChanged();
}

void UiFileController::openFile(const QString& path) {
    if (!core_) return;
    core_->openFile(path.toStdString());
    emit currentPathChanged();
}

void UiFileController::saveFile() {
    if (!core_) return;
    core_->saveFile();
    emit currentPathChanged();
}

void UiFileController::saveFileAs(const QString& path) {
    if (!core_) return;
    core_->saveFileAs(path.toStdString());
    emit currentPathChanged();
}
