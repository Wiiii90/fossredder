#include "ui/controllers/TransactionController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"

namespace ui {

TransactionController::TransactionController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString TransactionController::addTransaction(const QString& name,
                                              const QString& bookingDate,
                                              double amount,
                                              const QString& description,
                                              const QString& statementId,
                                              int status,
                                              const QString& actorId,
                                              bool allocatable,
                                              const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::TransactionController::addTransaction")) return {};
    try {
        return QString::fromStdString(core_->addTransaction(strings::toStdString(name),
                                                             strings::toStdString(bookingDate),
                                                             amount,
                                                             strings::toStdString(description),
                                                             statementId.toStdString(),
                                                             status,
                                                             actorId.toStdString(),
                                                             allocatable,
                                                             strings::toStdList(propertyIds)));
    } catch (...) {
        controllers::guard::reportException("ui::TransactionController::addTransaction");
    }
    return {};
}

void TransactionController::updateTransaction(const QString& id,
                                              const QString& name,
                                              const QString& bookingDate,
                                              double amount,
                                              const QString& description,
                                              const QString& statementId,
                                              int status,
                                              const QString& actorId,
                                              bool allocatable,
                                              const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::TransactionController::updateTransaction")) return;
    try {
        core_->updateTransaction(id.toStdString(),
                                 strings::toStdString(name),
                                 strings::toStdString(bookingDate),
                                 amount,
                                 strings::toStdString(description),
                                 statementId.toStdString(),
                                 status,
                                 actorId.toStdString(),
                                 allocatable,
                                 strings::toStdList(propertyIds));
    } catch (...) {
        controllers::guard::reportException("ui::TransactionController::updateTransaction");
    }
}

void TransactionController::deleteTransaction(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::TransactionController::deleteTransaction")) return;
    try {
        core_->deleteTransaction(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::TransactionController::deleteTransaction");
    }
}

}
