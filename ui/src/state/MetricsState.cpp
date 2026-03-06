#include "ui/state/MetricsState.h"

#include "ui/models/PropertyList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/ContractList.h"
#include "ui/payload/UiPayloadKeys.h"

namespace ui {

namespace {

constexpr double kZeroAmount = 0.0;

QHash<QString, QString> buildContractTypeById(const ContractList& contracts)
{
    QHash<QString, QString> out;
    const auto& rows = contracts.contracts();
    out.reserve(static_cast<int>(rows.size()));
    for (const auto& c : rows) {
        if (!c) continue;
        out.insert(QString::fromStdString(c->id), QString::fromStdString(c->type));
    }
    return out;
}

bool containsPropertyId(const std::vector<std::string>& propertyIds, const QString& propertyId)
{
    for (const auto& pid : propertyIds) {
        if (QString::fromStdString(pid) == propertyId) return true;
    }
    return false;
}

}

QStringList MetricsState::propertyContractTypes(const QString& propertyId, const TransactionList& transactions, const ContractList& contracts) const
{
    QStringList out;
    if (propertyId.isEmpty()) return out;
    const auto contractTypeById = buildContractTypeById(contracts);
    for (const auto& t : transactions.transactions()) {
        if (!t) continue;
        if (!containsPropertyId(t->propertyIds, propertyId) || t->contractId.empty()) continue;
        const QString contractType = contractTypeById.value(QString::fromStdString(t->contractId)).trimmed();
        if (!contractType.isEmpty() && !out.contains(contractType)) out.push_back(contractType);
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
    out[payload::keys::metrics::kTotal] = kZeroAmount;
    out[payload::keys::metrics::kAllocatable] = kZeroAmount;
    out[payload::keys::metrics::kNonAllocatable] = kZeroAmount;
    if (propertyId.isEmpty()) return out;

    const auto contractTypeById = buildContractTypeById(contracts);

    for (const auto& t : transactions.transactions()) {
        if (!t) continue;
        if (!containsPropertyId(t->propertyIds, propertyId)) continue;

        if (!contractType.isEmpty()) {
            const QString currentType = t->contractId.empty()
                ? QString()
                : contractTypeById.value(QString::fromStdString(t->contractId));
            if (currentType != contractType) continue;
        }

        out[payload::keys::metrics::kTotal] = out[payload::keys::metrics::kTotal].toDouble() + t->amount;
        if (t->allocatable) out[payload::keys::metrics::kAllocatable] = out[payload::keys::metrics::kAllocatable].toDouble() + t->amount;
        else out[payload::keys::metrics::kNonAllocatable] = out[payload::keys::metrics::kNonAllocatable].toDouble() + t->amount;
    }

    return out;
}

}
