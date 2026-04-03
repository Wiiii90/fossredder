#include "ui/state/MetricsState.h"

#include "core/application/PropertyMetricsService.h"

#include "ui/models/PropertyList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/ContractList.h"
#include "ui/payload/PayloadKeys.h"

namespace ui {

namespace {

QVariantMap toVariantMap(const core::application::PropertySums& sums)
{
    return {
        {payload::keys::metrics::kTotal, sums.total},
        {payload::keys::metrics::kAllocatable, sums.allocatable},
        {payload::keys::metrics::kNonAllocatable, sums.nonAllocatable}
    };
}

}

QStringList MetricsState::propertyContractTypes(const QString& propertyId, const TransactionList& transactions, const ContractList& contracts) const
{
    QStringList out;
    for (const auto& type : core::application::PropertyMetricsService::propertyContractTypes(
             propertyId.toStdString(), transactions.transactions(), contracts.contracts())) {
        const QString value = QString::fromStdString(type).trimmed();
        if (!value.isEmpty() && !out.contains(value)) out.push_back(value);
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
    return toVariantMap(core::application::PropertyMetricsService::propertySums(
        propertyId.toStdString(),
        contractType.toStdString(),
        transactions.transactions(),
        contracts.contracts()));
}

}
