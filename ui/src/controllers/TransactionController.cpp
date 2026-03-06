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
    return controllers::guard::invokeValue<QString>(core_, "ui::TransactionController::addTransaction", {}, [&]() {
        return QString::fromStdString(core_->addTransaction(strings::toStdString(name),
                                                             strings::toStdString(bookingDate),
                                                             amount,
                                                             strings::toStdString(description),
                                                             statementId.toStdString(),
                                                             status,
                                                             actorId.toStdString(),
                                                             allocatable,
                                                             strings::toStdList(propertyIds)));
    });
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
    controllers::guard::invokeVoid(core_, "ui::TransactionController::updateTransaction", [&]() {
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
    });
}

void TransactionController::deleteTransaction(const QString& id)
{
    controllers::guard::invokeVoid(core_, "ui::TransactionController::deleteTransaction", [&]() {
        core_->deleteTransaction(id.toStdString());
    });
}

}
