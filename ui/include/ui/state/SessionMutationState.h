#pragma once

#include <functional>

#include <QString>
#include <QStringList>

#include "core/models/DeletionImpact.h"

namespace ui {

class MetricsState;
class PropertyNameCatalog;
class SessionModels;
class FilterState;

class SessionMutationState {
public:
    using TransactionSumsNotifier = std::function<void(const QString& propertyId)>;

    static void applyDeletionImpact(const DeletionImpact& impact,
                                    SessionModels& models,
                                    FilterState& filters,
                                    MetricsState& metrics,
                                    PropertyNameCatalog& propertyNames);

    static void setTransactionPropertyIdsImmediate(const QString& transactionId,
                                                   const QStringList& propertyIds,
                                                   SessionModels& models,
                                                   MetricsState& metrics,
                                                   const TransactionSumsNotifier& notifyTransactionSums);
};

}
