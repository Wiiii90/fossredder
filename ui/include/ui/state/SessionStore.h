#pragma once

#include <QObject>
#include <QVariantMap>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "ui/models/TransactionFilter.h"
#include "ui/state/FilterState.h"
#include "ui/state/MetricsState.h"
#include "ui/state/PropertyNameCatalog.h"
#include "ui/state/SessionModels.h"

namespace ui {

class SessionStore : public QObject {
    Q_OBJECT

public:
    explicit SessionStore(QObject* parent = nullptr);

    SessionModels& models() noexcept { return models_; }
    const SessionModels& models() const noexcept { return models_; }

    void loadFromState(const AppState& state);
    TransactionFilter* statementTransactions(const QString& statementId, QObject* parent);
    TransactionFilter* propertyTransactions(const QString& propertyId, QObject* parent);
    QStringList propertyContractTypes(const QString& propertyId) const;
    QVariantMap propertyTransactionSums(const QString& propertyId, const QString& contractType = QString()) const;
    QString propertyName(const QString& id) const;
    void applyDeletionImpact(const DeletionImpact& impact);
    void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

signals:
    void selectionRefreshRequested();
    void transactionSumsUpdated(const QString& propertyId);

private:
    void bindModelSignals();
    void recomputeAllMetrics();
    void recomputeMetricsForRows(int firstRow, int lastRow);
    void notifyTransactionSumsForAllProperties();

    SessionModels models_;
    FilterState filters_;
    mutable MetricsState metrics_;
    PropertyNameCatalog propertyNames_;
};

}
