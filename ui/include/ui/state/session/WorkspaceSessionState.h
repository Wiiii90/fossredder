/**
 * @file ui/include/ui/state/session/WorkspaceSessionState.h
 * @brief Declares the UI session store that owns models, filters and derived metrics.
 */

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/application/storage/DeletionImpact.h"
#include "ui/viewmodels/booking/TransactionListModel.h"
#include "ui/viewmodels/booking/TransactionFilterModel.h"
#include "ui/state/filters/FilterState.h"
#include "ui/state/session/WorkspaceSessionModels.h"

namespace ui {

/**
 * @brief Owns the UI-facing session models and derived lookup/filter state.
 */
class SessionStore : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(SessionStore)
    QML_UNCREATABLE("SessionStore is exposed by the application context")
    Q_PROPERTY(int dataRevision READ dataRevision NOTIFY dataRevisionChanged)

public:
    /** @brief Creates the session store and connects derived-state recomputation. */
    explicit SessionStore(QObject* parent = nullptr);

    SessionModels& models() noexcept { return models_; }
    const SessionModels& models() const noexcept { return models_; }

    /** @brief Replaces all UI model data from a domain application state snapshot. */
    void loadFromState(const core::domain::catalog::WorkspaceCatalog& state);
    /** @brief Returns a live filter over transactions for a statement. */
    TransactionFilter* statementTransactions(const QString& statementId);
    /** @brief Returns a live filter over transactions for a property. */
    TransactionFilter* propertyTransactions(const QString& propertyId);
    /** @brief Applies domain deletion effects to session models. */
    void applyDeletionImpact(const core::domain::DeletionImpact& impact);
    /** @brief Applies an immediate property reassignment for a single transaction. */
    void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);
    int dataRevision() const noexcept { return dataRevision_; }

signals:
    void dataRevisionChanged();

private:
    void bindModelSignals();
    void bumpDataRevision();

    FilterState filters_;
    SessionModels models_;
    int dataRevision_ = 0;
};

}

