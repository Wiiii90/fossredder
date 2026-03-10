#include "ui/controllers/StorageController.h"

#include "ui/controllers/ControllerContracts.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"
#include "ui/text/Text.h"

namespace ui {

void StorageController::setLastError(const QString &error) {
  if (lastError_ == error)
    return;
  lastError_ = error;
  emit errorChanged();
}

bool StorageController::runCoreOperation(const char *context,
                                         const std::function<void()> &action) {
  return controllers::guard::invokeValue<bool>(core_, context, false, [&]() {
    action();
    return true;
  });
}

void StorageController::finishOperation(bool success,
                                        const QString &failureText,
                                        const QString &operation) {
  if (!success) {
    setLastError(failureText);
    emit operationFailed(operation, lastError_);
    return;
  }

  setLastError({});
  emit currentPathChanged();
  emit operationSucceeded(operation);
}

StorageController::StorageController(AppStateController *core, QObject *parent)
    : QObject(parent), core_(core) {}

QString StorageController::currentPath() const {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::storage::kCurrentPath, {},
      [&]() { return QString::fromStdString(core_->currentPath()); });
}

void StorageController::newFile(const QString &path) {
  const bool success =
      runCoreOperation(observability::origins::controller::storage::kNewFile,
                       [&]() { core_->newFile(strings::toEncodedPath(path)); });
  finishOperation(success, tr(ui::text::controllerErrors::kStorageCreateFailed),
                  controllers::contracts::operations::kNewFile);
}

void StorageController::openFile(const QString &path) {
  const bool success = runCoreOperation(
      observability::origins::controller::storage::kOpenFile,
      [&]() { core_->openFile(strings::toEncodedPath(path)); });
  finishOperation(success, tr(ui::text::controllerErrors::kStorageOpenFailed),
                  controllers::contracts::operations::kOpenFile);
}

void StorageController::saveFile() {
  const bool success =
      runCoreOperation(observability::origins::controller::storage::kSaveFile,
                       [&]() { core_->saveFile(); });
  finishOperation(success, tr(ui::text::controllerErrors::kStorageSaveFailed),
                  controllers::contracts::operations::kSaveFile);
}

void StorageController::saveFileAs(const QString &path) {
  const bool success = runCoreOperation(
      observability::origins::controller::storage::kSaveFileAs,
      [&]() { core_->saveFileAs(strings::toEncodedPath(path)); });
  finishOperation(success, tr(ui::text::controllerErrors::kStorageSaveAsFailed),
                  controllers::contracts::operations::kSaveFileAs);
}

} // namespace ui
