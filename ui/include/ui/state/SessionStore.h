/**
 * @file ui/include/ui/state/SessionStore.h
 * @brief Declares the UI session store that owns models, filters and derived metrics.
 */

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "ui/models/TransactionList.h"
#include "ui/models/TransactionFilter.h"
#include "ui/state/FilterState.h"
#include "ui/state/SessionModels.h"

namespace ui {

/**
 * @brief Owns the UI-facing session models and derived lookup/filter state.
 */
class SessionStore : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(SessionStore)
    QML_UNCREATABLE("SessionStore is exposed by the application context")

public:
    /** @brief Creates the session store and connects derived-state recomputation. */
    explicit SessionStore(QObject* parent = nullptr);

    SessionModels& models() noexcept { return models_; }
    const SessionModels& models() const noexcept { return models_; }

    /** @brief Replaces all UI model data from a domain application state snapshot. */
    void loadFromState(const core::domain::AppState& state);
    /** @brief Returns a live filter over transactions for a statement. */
    TransactionFilter* statementTransactions(const QString& statementId);
    /** @brief Returns a live filter over transactions for a property. */
    TransactionFilter* propertyTransactions(const QString& propertyId);
    /** @brief Applies domain deletion effects to session models. */
    void applyDeletionImpact(const core::domain::DeletionImpact& impact);
    /** @brief Applies an immediate property reassignment for a single transaction. */
    void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

private:
    void bindModelSignals();

    FilterState filters_;
    SessionModels models_;
};

}
