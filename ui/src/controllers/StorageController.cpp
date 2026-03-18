/**
 * @file ui/src/controllers/StorageController.cpp
 * @brief Implements UI-facing storage operations and error propagation.
 */

#include "ui/controllers/StorageController.h"

#include "ui/controllers/ControllerContracts.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"
#include "ui/text/Text.h"

namespace ui {

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
    emit operationFailed(operation, failureText);
    return;
  }

  emit operationSucceeded(operation);
}

StorageController::StorageController(core::application::AppStateFacade *core,
                                     QObject *parent)
    : QObject(parent), core_(core) {}

void StorageController::newFile(const QString &path) {
  const bool success =
      runCoreOperation(observability::origins::controller::storage::kNewFile,
                       [&]() { core_->newFile(strings::toEncodedPath(path)); });
  finishOperation(success, ui::text::controllerErrors::storageCreateFailed(),
                  controllers::contracts::operations::kNewFile);
}

void StorageController::openFile(const QString &path) {
  const bool success = runCoreOperation(
      observability::origins::controller::storage::kOpenFile,
      [&]() { core_->openFile(strings::toEncodedPath(path)); });
  finishOperation(success, ui::text::controllerErrors::storageOpenFailed(),
                  controllers::contracts::operations::kOpenFile);
}

void StorageController::saveFile() {
  const bool success =
      runCoreOperation(observability::origins::controller::storage::kSaveFile,
                       [&]() { core_->saveFile(); });
  finishOperation(success, ui::text::controllerErrors::storageSaveFailed(),
                  controllers::contracts::operations::kSaveFile);
}

void StorageController::saveFileAs(const QString &path) {
  const bool success = runCoreOperation(
      observability::origins::controller::storage::kSaveFileAs,
      [&]() { core_->saveFileAs(strings::toEncodedPath(path)); });
  finishOperation(success, ui::text::controllerErrors::storageSaveAsFailed(),
                  controllers::contracts::operations::kSaveFileAs);
}

} // namespace ui
