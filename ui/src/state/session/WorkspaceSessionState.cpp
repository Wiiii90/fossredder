/**
 * @file ui/src/state/SessionState.cpp
 * @brief Implements the UI session store and its derived metrics maintenance.
 */

#include "ui/state/session/WorkspaceSessionState.h"

#include "core/application/import/transaction/AmountParser.h"
#include "ui/viewmodels/booking/TransactionFilterModel.h"
#include "ui/state/mutation/SessionMutationState.h"
#include "ui/adapters/core/WorkspaceRowProjector.h"

#include <cmath>
#include <optional>

namespace ui {

namespace {

std::optional<double> parseFlexibleAmount(const QString& value)
{
    QString raw = value;
    raw = raw.trimmed();
    raw.remove(QChar(0x00A0));
    raw.remove(QChar(0x202F));
    raw.remove(QLatin1Char(' '));
    if (raw.isEmpty()) {
        return std::nullopt;
    }

    bool okDirect = false;
    const double direct = raw.toDouble(&okDirect);
    if (okDirect && std::isfinite(direct)) {
        return direct;
    }

    int decimalPos = -1;
    for (int i = raw.size() - 1; i >= 0; --i) {
        const QChar ch = raw.at(i);
        if (ch == QLatin1Char('.') || ch == QLatin1Char(',')) {
            decimalPos = i;
            break;
        }
    }

    QString canonical;
    canonical.reserve(raw.size());
    for (int i = 0; i < raw.size(); ++i) {
        const QChar ch = raw.at(i);
        if (ch.isDigit()) {
            canonical.append(ch);
            continue;
        }
        if ((ch == QLatin1Char('-') || ch == QLatin1Char('+')) && i == 0) {
            canonical.append(ch);
            continue;
        }
        if (ch == QLatin1Char('.') || ch == QLatin1Char(',')) {
            if (i == decimalPos) {
                canonical.append(QLatin1Char('.'));
            }
            continue;
        }
        return std::nullopt;
    }

    if (canonical.isEmpty() || canonical == QLatin1String("-") || canonical == QLatin1String("+")) {
        return std::nullopt;
    }

    bool okCanonical = false;
    const double parsed = canonical.toDouble(&okCanonical);
    if (!okCanonical || !std::isfinite(parsed)) {
        return std::nullopt;
    }
    return parsed;
}

} // namespace

SessionState::SessionState(QObject* parent)
    : QObject(parent)
    , filters_(this)
    , models_(this)
{
    bindModelSignals();
}

void SessionState::bindModelSignals()
{
}

void SessionState::bumpDataRevision()
{
    ++dataRevision_;
    emit dataRevisionChanged();
}

void SessionState::loadFromState(const core::domain::catalog::WorkspaceCatalog& state)
{
    filters_.clear();
    models_.loadFromState(state);
    bumpDataRevision();
}

TransactionFilter* SessionState::statementTransactions(const QString& statementId)
{
    return filters_.statementTransactions(statementId, models_.transactions());
}

TransactionFilter* SessionState::propertyTransactions(const QString& propertyId)
{
    return filters_.propertyTransactions(propertyId, models_.transactions());
}

void SessionState::applyDeletionImpact(const DeletionImpact& impact)
{
    SessionMutationState::applyDeletionImpact(impact,
                                              models_,
                                              filters_);
    models_.refreshContractTypes();
    bumpDataRevision();
}

void SessionState::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    SessionMutationState::setTransactionPropertyIdsImmediate(txId, propertyIds, models_);
    bumpDataRevision();
}

QVariantList SessionState::normalizeStrings(const QVariantList& values) const
{
    return SessionMutationState::normalizeStrings(values);
}

QVariantMap SessionState::transactionDraft(const QVariantMap& draft,
                                           const QVariantList& contractRows,
                                           const QVariantMap& changes) const
{
    return SessionMutationState::transactionDraft(draft, contractRows, changes);
}

QVariantList SessionState::addUniqueTrimmed(const QVariantList& values, const QString& value) const
{
    return SessionMutationState::addUniqueTrimmed(values, value);
}

QVariantList SessionState::removeAt(const QVariantList& values, int index) const
{
    return SessionMutationState::removeAt(values, index);
}

QVariantList SessionState::removeString(const QVariantList& values, const QString& value) const
{
    return SessionMutationState::removeString(values, value);
}

QVariantList SessionState::insertAt(const QVariantList& values, int index, const QVariant& value) const
{
    return SessionMutationState::insertAt(values, index, value);
}

QVariantList SessionState::pruneAndAppendMissing(const QVariantList& preferredIds, const QVariantList& availableIds) const
{
    return ui::pruneAndAppendMissing(preferredIds, availableIds);
}

int SessionState::indexOfId(const QVariantList& rows, const QString& id) const
{
    return ui::indexOfId(rows, id);
}

int SessionState::indexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value) const
{
    return ui::indexOfKeyValue(rows, key, value);
}

int SessionState::indexOfString(const QVariantList& values, const QString& value) const
{
    return ui::indexOfString(values, value);
}

int SessionState::normalizedIndex(int index, int count) const
{
    return ui::normalizedIndex(index, count);
}

int SessionState::wrappedIndex(int index, int count) const
{
    return ui::wrappedIndex(index, count);
}

QString SessionState::wrappedIdAt(const QVariantList& rows, int index) const
{
    return ui::wrappedIdAt(rows, index);
}

QString SessionState::navigatedId(const QVariantList& rows,
                                  const QString& currentId,
                                  int delta,
                                  int defaultIndex) const
{
    return ui::navigatedId(rows, currentId, delta, defaultIndex);
}

QVariantList SessionState::displayRowsWithEmpty(const QVariantList& rows,
                                                const QString& emptyDisplay,
                                                const QString& displayKey) const
{
    return ui::displayRowsWithEmpty(rows, emptyDisplay, displayKey);
}

QVariantList SessionState::rowIds(const QVariantList& rows, const QString& idKey) const
{
    return ui::rowIds(rows, idKey);
}

QVariantList SessionState::orderedRowsByIds(const QVariantList& rows,
                                            const QVariantList& orderIds,
                                            const QString& idKey) const
{
    return ui::orderedRowsByIds(rows, orderIds, idKey);
}

QVariantMap SessionState::mapWithKeyValue(const QVariantMap& base, const QString& key, const QVariant& value) const
{
    return ui::mapWithKeyValue(base, key, value);
}

QVariantMap SessionState::emptyTransactionDraft() const
{
    return SessionMutationState::emptyTransactionDraft();
}

QVariantMap SessionState::normalizeTransactionDraft(const QVariantMap& tx) const
{
    return SessionMutationState::normalizeTransactionDraft(tx);
}

QVariantList SessionState::normalizeTransactionDrafts(const QVariantList& values) const
{
    return SessionMutationState::normalizeTransactionDrafts(values);
}

bool SessionState::transactionDraftHasContent(const QVariantMap& tx) const
{
    return SessionMutationState::transactionDraftHasContent(tx);
}

QVariantMap SessionState::createDraftListState(const QVariantList& drafts,
                                               int currentIndex,
                                               const QVariantMap& emptyDraft) const
{
    return SessionMutationState::createDraftListState(drafts, currentIndex, emptyDraft);
}

QVariantMap SessionState::insertDraftAfterCurrent(const QVariantList& drafts,
                                                  int currentIndex,
                                                  const QVariantMap& emptyDraft) const
{
    return SessionMutationState::insertDraftAfterCurrent(drafts, currentIndex, emptyDraft);
}

QVariantMap SessionState::removeDraftAt(const QVariantList& drafts,
                                        int currentIndex,
                                        const QVariantMap& emptyDraft) const
{
    return SessionMutationState::removeDraftAt(drafts, currentIndex, emptyDraft);
}

QVariantMap SessionState::setCurrentDraft(const QVariantList& drafts,
                                          int currentIndex,
                                          const QVariantMap& draft,
                                          const QVariantMap& emptyDraft) const
{
    return SessionMutationState::setCurrentDraft(drafts, currentIndex, draft, emptyDraft);
}

QVariantMap SessionState::currentDraftState(const QVariantList& drafts,
                                            int currentIndex,
                                            const QVariantMap& emptyDraft) const
{
    return SessionMutationState::currentDraftState(drafts, currentIndex, emptyDraft);
}

QVariantMap SessionState::resolveSelectionState(const QVariantList& rows,
                                                int currentIndex,
                                                const QString& selectedId,
                                                const QString& idKey) const
{
    return ui::resolveSelectionState(rows, currentIndex, selectedId, idKey);
}

QVariantList SessionState::orderWithInsertedId(const QVariantList& currentOrder,
                                               const QVariantList& availableIds,
                                               const QString& insertedId,
                                               int insertAfterIndex) const
{
    return ui::orderWithInsertedId(currentOrder, availableIds, insertedId, insertAfterIndex);
}

QVariantMap SessionState::orderedRowsState(const QVariantList& rows,
                                           const QVariantList& preferredOrder,
                                           const QString& idKey) const
{
    return ui::orderedRowsState(rows, preferredOrder, idKey);
}

QVariantMap SessionState::orderedSelectionState(const QVariantList& rows,
                                                const QVariantList& preferredOrder,
                                                int currentIndex,
                                                const QString& selectedId,
                                                const QString& idKey) const
{
    return ui::orderedSelectionState(rows, preferredOrder, currentIndex, selectedId, idKey);
}

QVariantMap SessionState::navigateSelectionState(const QVariantList& rows,
                                                 int currentIndex,
                                                 const QString& selectedId,
                                                 int delta,
                                                 int defaultIndex,
                                                 const QString& idKey) const
{
    return ui::navigateSelectionState(rows, currentIndex, selectedId, delta, defaultIndex, idKey);
}

QVariantMap SessionState::deleteReselectionState(const QVariantList& rows,
                                                 const QVariantList& preferredOrder,
                                                 int currentIndex,
                                                 const QString& removedId,
                                                 const QString& idKey) const
{
    return ui::deleteReselectionState(rows, preferredOrder, currentIndex, removedId, idKey);
}

QString SessionState::deleteNextSelectionId(const QVariantList& rows,
                                            const QString& removedId,
                                            int defaultIndex,
                                            const QString& idKey) const
{
    return ui::deleteNextSelectionId(rows, removedId, defaultIndex, idKey);
}

QVariantMap SessionState::basicFormState(const QString& name,
                                         const QVariantList& aliases,
                                         const QVariantList& selectedIds) const
{
    return ui::basicFormState(name, aliases, selectedIds);
}

QVariantMap SessionState::contractFormState(const QString& name,
                                            const QString& type,
                                            const QVariantList& actorIds,
                                            const QVariantList& propertyIds,
                                            const QVariantList& aliases) const
{
    return ui::contractFormState(name, type, actorIds, propertyIds, aliases);
}

double SessionState::amountForTransactionCommit(const QVariant& rawAmount,
                                                const QString& transactionId,
                                                double fallbackAmount) const
{
    if (rawAmount.userType() == QMetaType::Double) {
        const double numeric = rawAmount.toDouble();
        if (std::isfinite(numeric)) {
            return numeric;
        }
    }

    const auto parsed =
        core::application::importing::transaction::parseAmountString(rawAmount.toString().toStdString());
    if (parsed && std::isfinite(*parsed)) {
        return *parsed;
    }

    if (const auto flexible = parseFlexibleAmount(rawAmount.toString())) {
        return *flexible;
    }

    if (!transactionId.isEmpty()) {
        const int row = models_.transactions().findRowById(transactionId);
        if (row >= 0) {
            const auto& rows = models_.transactions().transactions();
            const auto& tx = rows.at(static_cast<std::size_t>(row));
            if (tx) {
                const double persistedAmount = tx->amount();
                if (std::isfinite(persistedAmount)) {
                    return persistedAmount;
                }
            }
        }
    }

    return std::isfinite(fallbackAmount) ? fallbackAmount : 0.0;
}

}

