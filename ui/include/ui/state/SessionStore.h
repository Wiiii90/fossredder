/**
 * @file ui/include/ui/state/SessionStore.h
 * @brief Declares the UI session store that owns models, filters and derived metrics.
 */

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

/**
 * @brief Owns the UI-facing session models and derived lookup/filter state.
 */
class SessionStore : public QObject {
    Q_OBJECT

public:
    /** @brief Creates the session store and connects derived-state recomputation. */
    explicit SessionStore(QObject* parent = nullptr);

    SessionModels& models() noexcept { return models_; }
    const SessionModels& models() const noexcept { return models_; }

    /** @brief Replaces all UI model data from a domain application state snapshot. */
    void loadFromState(const core::domain::AppState& state);
    /** @brief Returns a live filter over transactions for a statement. */
    TransactionFilter* statementTransactions(const QString& statementId, QObject* parent);
    /** @brief Returns a live filter over transactions for a property. */
    TransactionFilter* propertyTransactions(const QString& propertyId, QObject* parent);
    /** @brief Returns the currently observed contract types for a property. */
    QStringList propertyContractTypes(const QString& propertyId) const;
    /** @brief Returns cached transaction sums for a property and optional contract type. */
    QVariantMap propertyTransactionSums(const QString& propertyId, const QString& contractType = QString()) const;
    /** @brief Resolves a property id to the cached display name. */
    QString propertyName(const QString& id) const;
    /** @brief Applies domain deletion effects to session models and derived caches. */
    void applyDeletionImpact(const core::domain::DeletionImpact& impact);
    /** @brief Applies an immediate property reassignment for a single transaction. */
    void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

signals:
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
