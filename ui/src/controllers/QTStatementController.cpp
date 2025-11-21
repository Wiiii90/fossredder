#include "ui/controllers/QTStatementController.h"
#include <QVariant>
#include <QList>
#include <QStringList>

#include "core/controllers/StatementController.h"
#include "core/models/Transaction.h"

QTStatementController::QTStatementController(std::shared_ptr<StatementController> coreController, QObject* parent)
    : QObject(parent), coreController_(std::move(coreController)) {}

void QTStatementController::importStatement(const QString& path) {
    if (!coreController_) return;
    std::string p = path.toStdString();
    try {
        auto data = coreController_->importStatement(p);
        QList<QVariant> list;
        emit transactionsExtracted(list);
    } catch (...) {
    }
}
