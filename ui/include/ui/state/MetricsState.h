#pragma once

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <functional>

#include "ui/models/PropertyList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/ContractList.h"

namespace ui {

class MetricsState {
public:
    QStringList propertyContractTypes(const QString& propertyId, const TransactionList& transactions, const ContractList& contracts) const;
    QVariantMap propertyTransactionSums(const QString& propertyId,
                                        const QString& contractType,
                                        const TransactionList& transactions,
                                        const ContractList& contracts) const;

    void recomputeAll(const PropertyList& properties,
                      const TransactionList& transactions,
                      const ContractList& contracts,
                      const std::function<void(const QString&)>& notifyChanged) const;

    void recomputeProperty(const QString& propertyId,
                           const TransactionList& transactions,
                           const ContractList& contracts,
                           const std::function<void(const QString&)>& notifyChanged) const;

    void clearCache() const;
    void removePropertyCache(const QString& propertyId) const;

private:
    QVariantMap computePropertySums(const QString& propertyId,
                                    const QString& contractType,
                                    const TransactionList& transactions,
                                    const ContractList& contracts) const;

    mutable QHash<QString, QVariantMap> propertySumsCache_;
};

}
