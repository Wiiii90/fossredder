#include "ui/controllers/QTStatementController.h"
#include <QVariant>
#include <QList>
#include <QStringList>
#include <QFile>

#include "core/controllers/StatementController.h"
#include "core/models/Transaction.h"

QTStatementController::QTStatementController(std::shared_ptr<StatementController> coreController, QObject* parent)
    : QObject(parent), coreController_(std::move(coreController)) {}

void QTStatementController::importStatement(const QString& path) {
    if (!coreController_) return;

    const QByteArray nativePath = QFile::encodeName(path);
    std::string p(nativePath.constData(), static_cast<size_t>(nativePath.size()));

    try {
        auto data = coreController_->importStatement(p);
        QList<QVariant> list;
        emit transactionsExtracted(list);
    } catch (...) {
    }
}
