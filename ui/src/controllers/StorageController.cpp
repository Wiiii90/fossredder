/**
 * @file ui/src/controllers/StorageController.cpp
 * @brief Implements UI-facing storage operations and error propagation.
 */

#include "ui/controllers/StorageController.h"

#include "ui/config/Defaults.h"
#include "ui/observability/Origins.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"
#include "ui/text/Text.h"

namespace ui {

bool StorageController::runCoreOperation(const char* context,
                                         const std::function<void()>& action)
{
    return ui::util::guard::invokeValue<bool>(core_, context, false, [&]() {
        action();
        return true;
    });
}

void StorageController::finishOperation(bool success,
                                        const QString& failureText,
                                        const QString& operation)
{
    if (!success) {
        emit operationFailed(operation, failureText);
        return;
    }

    emit operationSucceeded(operation);
}

StorageController::StorageController(core::application::AppStateFacade* core,
                                     QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

void StorageController::newFile(const QString& path)
{
    const bool success =
        runCoreOperation(observability::origins::controller::storage::kNewFile,
                         [&]() { core_->newFile(strings::toEncodedPath(path)); });
    finishOperation(success, ui::text::controllerErrors::storageCreateFailed(),
                  ui::config::operationKeys::kNewFile);
}

void StorageController::openFile(const QString& path)
{
    const bool success = runCoreOperation(
        observability::origins::controller::storage::kOpenFile,
        [&]() { core_->openFile(strings::toEncodedPath(path)); });
    finishOperation(success, ui::text::controllerErrors::storageOpenFailed(),
                  ui::config::operationKeys::kOpenFile);
}

void StorageController::saveFile()
{
    const bool success =
        runCoreOperation(observability::origins::controller::storage::kSaveFile,
                         [&]() { core_->saveFile(); });
    finishOperation(success, ui::text::controllerErrors::storageSaveFailed(),
                  ui::config::operationKeys::kSaveFile);
}

void StorageController::saveFileAs(const QString& path)
{
    const bool success = runCoreOperation(
        observability::origins::controller::storage::kSaveFileAs,
        [&]() { core_->saveFileAs(strings::toEncodedPath(path)); });
    finishOperation(success, ui::text::controllerErrors::storageSaveAsFailed(),
                  ui::config::operationKeys::kSaveFileAs);
}

} // namespace ui
