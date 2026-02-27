#include "ui/controllers/TransactionController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "ui/controllers/ControllerStrings.h"
#include "core/models/Transaction.h"

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
    if (!core_) return {};
    if (statementId.trimmed().isEmpty()) return {};

    auto tx = std::make_shared<Transaction>();
    tx->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    tx->name = strings::toStdString(name);
    tx->bookingDate = strings::toStdString(bookingDate);
    tx->amount = amount;
    tx->description = strings::toStdString(description);
    tx->statementId = statementId.toStdString();
    tx->status = static_cast<Transaction::Status>(status);
    tx->actorId = actorId.toStdString();
    tx->allocatable = allocatable;
    tx->propertyIds = strings::toStdList(propertyIds);
    tx->valuta.clear();
    tx->actorProposal.clear();
    tx->metadata.clear();
    tx->proofImagePath.clear();

    core_->mutableState().transactions.push_back(tx);
    core_->notifyState();
    core_->commit();
    return QString::fromStdString(tx->id);
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
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& t : core_->mutableState().transactions) {
        if (!t || t->id != sid) continue;
        t->name = strings::toStdString(name);
        t->bookingDate = strings::toStdString(bookingDate);
        t->amount = amount;
        t->description = strings::toStdString(description);
        t->statementId = statementId.toStdString();
        t->status = static_cast<Transaction::Status>(status);
        t->actorId = actorId.toStdString();
        t->allocatable = allocatable;
        t->propertyIds = strings::toStdList(propertyIds);
        core_->notifyState();
        core_->commit();
        return;
    }
}

void TransactionController::deleteTransaction(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& v = core_->mutableState().transactions;
    v.erase(std::remove_if(v.begin(), v.end(), [&](const auto& t) { return t && t->id == sid; }), v.end());
    core_->notifyState();
    core_->commit();
}

}
