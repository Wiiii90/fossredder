#include "ui/controllers/UiStatementController.h"

#include <QFile>

#include "core/controllers/StatementController.h"

UiStatementController::UiStatementController(std::shared_ptr<StatementController> coreController, QObject* parent)
    : QObject(parent), coreController_(std::move(coreController)) {}

void UiStatementController::importStatement(const QString& path) {
    if (!coreController_) return;

    const QByteArray nativePath = QFile::encodeName(path);
    std::string p(nativePath.constData(), static_cast<size_t>(nativePath.size()));

    try {
        (void)coreController_->importStatement(p);
        emit transactionsExtracted({});
    } catch (...) {
    }
}
