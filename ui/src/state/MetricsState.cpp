#include "ui/state/MetricsState.h"

#include "ui/models/PropertyList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/ContractList.h"

namespace ui {

QStringList MetricsState::propertyContractTypes(const QString& propertyId, const TransactionList& transactions, const ContractList& contracts) const
{
    QStringList out;
    if (propertyId.isEmpty()) return out;
    for (const auto& t : transactions.transactions()) {
        if (!t) continue;
        bool contains = false;
        for (const auto& pid : t->propertyIds) {
            if (QString::fromStdString(pid) == propertyId) { contains = true; break; }
        }
        if (!contains || t->contractId.empty()) continue;
        for (const auto& c : contracts.contracts()) {
            if (!c) continue;
            if (QString::fromStdString(c->id) != QString::fromStdString(t->contractId)) continue;
            const QString contractType = QString::fromStdString(c->type).trimmed();
            if (!contractType.isEmpty() && !out.contains(contractType)) out.push_back(contractType);
            break;
        }
    }
    return out;
}

QVariantMap MetricsState::propertyTransactionSums(const QString& propertyId,
                                                  const QString& contractType,
                                                  const TransactionList& transactions,
                                                  const ContractList& contracts) const
{
    if (propertyId.isEmpty()) return {};
    if (contractType.isEmpty() && propertySumsCache_.contains(propertyId)) return propertySumsCache_.value(propertyId);

    QVariantMap out = computePropertySums(propertyId, contractType, transactions, contracts);
    if (contractType.isEmpty()) propertySumsCache_.insert(propertyId, out);
    return out;
}

void MetricsState::recomputeAll(const PropertyList& properties,
                                const TransactionList& transactions,
                                const ContractList& contracts,
                                const std::function<void(const QString&)>& notifyChanged) const
{
    propertySumsCache_.clear();
    for (const auto& p : properties.properties()) {
        if (!p) continue;
        recomputeProperty(QString::fromStdString(p->id), transactions, contracts, notifyChanged);
    }
}

void MetricsState::recomputeProperty(const QString& propertyId,
                                     const TransactionList& transactions,
                                     const ContractList& contracts,
                                     const std::function<void(const QString&)>& notifyChanged) const
{
    if (propertyId.isEmpty()) return;
    QVariantMap out = computePropertySums(propertyId, QString(), transactions, contracts);
    propertySumsCache_.insert(propertyId, out);
    if (notifyChanged) notifyChanged(propertyId);
}

void MetricsState::clearCache() const
{
    propertySumsCache_.clear();
}

void MetricsState::removePropertyCache(const QString& propertyId) const
{
    if (propertyId.isEmpty()) return;
    propertySumsCache_.remove(propertyId);
}

QVariantMap MetricsState::computePropertySums(const QString& propertyId,
                                              const QString& contractType,
                                              const TransactionList& transactions,
                                              const ContractList& contracts) const
{
    QVariantMap out;
    out["total"] = 0.0;
    out["allocatable"] = 0.0;
    out["nonAllocatable"] = 0.0;
    if (propertyId.isEmpty()) return out;

    for (const auto& t : transactions.transactions()) {
        if (!t) continue;
        bool contains = false;
        for (const auto& pid : t->propertyIds) {
            if (QString::fromStdString(pid) == propertyId) { contains = true; break; }
        }
        if (!contains) continue;

        if (!contractType.isEmpty()) {
            QString currentType;
            if (!t->contractId.empty()) {
                for (const auto& c : contracts.contracts()) {
                    if (!c) continue;
                    if (QString::fromStdString(c->id) != QString::fromStdString(t->contractId)) continue;
                    currentType = QString::fromStdString(c->type);
                    break;
                }
            }
            if (currentType != contractType) continue;
        }

        out["total"] = out["total"].toDouble() + t->amount;
        if (t->allocatable) out["allocatable"] = out["allocatable"].toDouble() + t->amount;
        else out["nonAllocatable"] = out["nonAllocatable"].toDouble() + t->amount;
    }

    return out;
}

}
