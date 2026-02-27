#include "ui/controllers/TransactionController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "core/models/Transaction.h"

namespace {
std::string q2s(const QString& s)
{
    const auto u8 = s.toUtf8();
    return std::string(u8.constData(), static_cast<size_t>(u8.size()));
}

std::vector<std::string> toStdIds(const QStringList& ids)
{
    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(ids.size()));
    for (const auto& id : ids) out.push_back(id.toStdString());
    return out;
}
}

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
    tx->name = q2s(name);
    tx->bookingDate = q2s(bookingDate);
    tx->amount = amount;
    tx->description = q2s(description);
    tx->statementId = statementId.toStdString();
    tx->status = static_cast<Transaction::Status>(status);
    tx->actorId = actorId.toStdString();
    tx->allocatable = allocatable;
    tx->propertyIds = toStdIds(propertyIds);
    tx->valuta.clear();
    tx->actorProposal.clear();
    tx->metadata.clear();
    tx->proofImagePath.clear();

    core_->mutableState().transactions.push_back(tx);
    core_->notifyState();
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
        t->name = q2s(name);
        t->bookingDate = q2s(bookingDate);
        t->amount = amount;
        t->description = q2s(description);
        t->statementId = statementId.toStdString();
        t->status = static_cast<Transaction::Status>(status);
        t->actorId = actorId.toStdString();
        t->allocatable = allocatable;
        t->propertyIds = toStdIds(propertyIds);
        core_->notifyState();
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
}
