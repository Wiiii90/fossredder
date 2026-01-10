#include "ui/state/UiDataSession.h"
#include <QDebug>

UiDataSession::UiDataSession(QObject* parent)
    : QObject(parent)
    , actors_(this)
    , properties_(this)
    , contracts_(this)
    , statements_(this)
    , transactions_(this)
    , selectedActor_(this)
    , selectedProperty_(this)
    , selectedContract_(this)
    , selectedStatement_(this)
    , selectedTransaction_(this)
{
}

void UiDataSession::loadFromState(const AppState& state)
{
    actors_.setActors(state.actors);
    properties_.setProperties(state.properties);
    contracts_.setContracts(state.contracts);
    statements_.setStatements(state.statements);
    transactions_.setTransactions(state.transactions);

    // debug logging to help track when properties arrive
    qDebug() << "UiDataSession::loadFromState: properties=" << properties_.rowCount();

    refreshSelectedActor();
    refreshSelectedProperty();
    refreshSelectedContract();
    refreshSelectedStatement();
    refreshSelectedTransaction();
}

void UiDataSession::setSelectedActorId(const QString& id)
{
    if (selectedActorId_ == id) return;
    selectedActorId_ = id;
    refreshSelectedActor();
    emit selectedActorIdChanged();
}

void UiDataSession::setSelectedPropertyId(const QString& id)
{
    if (selectedPropertyId_ == id) return;
    selectedPropertyId_ = id;
    refreshSelectedProperty();
    emit selectedPropertyIdChanged();
}

void UiDataSession::setSelectedContractId(const QString& id)
{
    if (selectedContractId_ == id) return;
    selectedContractId_ = id;
    refreshSelectedContract();
    emit selectedContractIdChanged();
}

void UiDataSession::setSelectedStatementId(const QString& id)
{
    if (selectedStatementId_ == id) return;
    selectedStatementId_ = id;
    refreshSelectedStatement();
    emit selectedStatementIdChanged();
}

void UiDataSession::setSelectedTransactionId(const QString& id)
{
    if (selectedTransactionId_ == id) return;
    selectedTransactionId_ = id;
    refreshSelectedTransaction();
    emit selectedTransactionIdChanged();
}

void UiDataSession::refreshSelectedActor()
{
    if (selectedActorId_.isEmpty()) {
        selectedActor_.clear();
        return;
    }

    for (const auto& a : actors_.actors()) {
        if (a && QString::fromStdString(a->id) == selectedActorId_) {
            selectedActor_.setActor(QString::fromStdString(a->id),
                                    QString::fromStdString(a->name),
                                    QString::fromStdString(a->type),
                                    QString::fromStdString(a->description));
            return;
        }
    }

    selectedActor_.clear();
}

void UiDataSession::refreshSelectedProperty()
{
    if (selectedPropertyId_.isEmpty()) {
        selectedProperty_.clear();
        return;
    }

    for (const auto& p : properties_.properties()) {
        if (p && QString::fromStdString(p->id) == selectedPropertyId_) {
            selectedProperty_.setProperty(QString::fromStdString(p->id),
                                          QString::fromStdString(p->name),
                                          QString::fromStdString(p->address),
                                          QString::fromStdString(p->description));
            return;
        }
    }

    selectedProperty_.clear();
}

void UiDataSession::refreshSelectedContract()
{
    if (selectedContractId_.isEmpty()) {
        selectedContract_.clear();
        return;
    }

    for (const auto& c : contracts_.contracts()) {
        if (c && QString::fromStdString(c->id) == selectedContractId_) {
            QStringList actorIds;
            for (const auto& aid : c->actorIds) actorIds.push_back(QString::fromStdString(aid));

            QStringList propertyIds;
            for (const auto& pid : c->propertyIds) propertyIds.push_back(QString::fromStdString(pid));

            selectedContract_.setContract(QString::fromStdString(c->id),
                                          QString::fromStdString(c->name),
                                          QString::fromStdString(c->type),
                                          QString::fromStdString(c->description),
                                          actorIds,
                                          propertyIds);
            return;
        }
    }

    selectedContract_.clear();
}

void UiDataSession::refreshSelectedStatement()
{
    if (selectedStatementId_.isEmpty()) {
        selectedStatement_.clear();
        return;
    }

    for (const auto& s : statements_.statements()) {
        if (s && QString::fromStdString(s->id) == selectedStatementId_) {
            selectedStatement_.setStatement(QString::fromStdString(s->id),
                                            QString::fromStdString(s->name));
            return;
        }
    }

    selectedStatement_.clear();
}

void UiDataSession::refreshSelectedTransaction()
{
    if (selectedTransactionId_.isEmpty()) {
        selectedTransaction_.clear();
        return;
    }

    for (const auto& t : transactions_.transactions()) {
        if (t && QString::fromStdString(t->id) == selectedTransactionId_) {
            // build property id list for UI
            QStringList propIds;
            for (const auto& pid : t->propertyIds) propIds.push_back(QString::fromStdString(pid));

            selectedTransaction_.setTransactionWithProperties(QString::fromStdString(t->id),
                                                            QString::fromStdString(t->name),
                                                            QString::fromStdString(t->bookingDate),
                                                            t->amount,
                                                            QString::fromStdString(t->description),
                                                            QString::fromStdString(t->statementId),
                                                            QString::fromStdString(t->actorId),
                                                            QString::fromStdString(t->actorProposal),
                                                            propIds,
                                                            t->allocatable);
            return;
        }
    }

    selectedTransaction_.clear();
}

QVariantList UiDataSession::transactionIdsForStatement(const QString& statementId) const
{
    QVariantList out;
    if (statementId.isEmpty()) return out;

    for (const auto& t : transactions_.transactions()) {
        if (!t) continue;
        if (QString::fromStdString(t->statementId) == statementId) {
            out.push_back(QString::fromStdString(t->id));
        }
    }
    return out;
}

QObject* UiDataSession::transactionsForStatement(const QString& statementId)
{
    if (statementId.isEmpty()) return nullptr;

    const auto key = statementId;
    if (txByStatement_.contains(key)) {
        return txByStatement_.value(key);
    }

    auto* proxy = new TransactionFilterModel(this);
    proxy->setSourceModel(&transactions_);
    proxy->setStatementId(statementId);
    txByStatement_.insert(key, proxy);
    return proxy;
}

void UiDataSession::applyTransactionUpdates(const std::vector<std::string>& ids, const AppState& state)
{
    if (ids.empty()) return;
    for (const auto& sid : ids) {
        const QString qid = QString::fromStdString(sid);
        // If this transaction is currently being edited in the UI, skip applying the incremental update
        if (editingTransactions_.contains(qid)) continue;
        int row = transactions_.findRowById(qid);
        if (row < 0) continue;
        // locate updated transaction in provided AppState
        for (const auto& t : state.transactions) {
            if (!t) continue;
            if (QString::fromStdString(t->id) == qid) {
                // Only update model row if values actually changed to avoid UI flicker
                const auto current = transactions_.transactions().at(static_cast<size_t>(row));
                bool changed = false;
                if (!current) changed = true;
                else {
                    if (current->allocatable != t->allocatable) changed = true;
                    else if (current->status != t->status) changed = true;
                    else if (current->amount != t->amount) changed = true;
                    else if (current->name != t->name) changed = true;
                    else if (current->bookingDate != t->bookingDate) changed = true;
                    else if (current->description != t->description) changed = true;
                    else if (current->propertyIds != t->propertyIds) changed = true;
                }
                if (changed) {
                    transactions_.setTransactionAt(row, t);
                    // if this transaction is currently selected, refresh selectedTransaction to keep UI in sync
                    if (selectedTransactionId_ == qid) refreshSelectedTransaction();
                }
                // debug: log propertyIds mismatch
                if (current && current->propertyIds != t->propertyIds) {
                    QStringList cur;
                    for (const auto& p : current->propertyIds) cur << QString::fromStdString(p);
                    QStringList upd;
                    for (const auto& p : t->propertyIds) upd << QString::fromStdString(p);
                    qDebug() << "applyTransactionUpdates: propIds changed for" << qid << "from" << cur << "to" << upd;
                }
                break;
            }
        }
    }
}

void UiDataSession::applyDeletionImpact(const DeletionImpact& impact)
{
    // Remove deleted transactions from model
    for (const auto& tid : impact.deletedTransactionIds) {
        const QString qid = QString::fromStdString(tid);
        // If currently being edited, defer actual removal
        if (editingTransactions_.contains(qid)) {
            pendingDeletionTransactions_.insert(qid);
            continue;
        }
        int row = transactions_.findRowById(qid);
        if (row >= 0) transactions_.removeAt(row);
        if (selectedTransactionId_ == qid) {
            selectedTransactionId_.clear();
            emit selectedTransactionIdChanged();
            selectedTransaction_.clear();
        }
    }

    // Remove deleted statements from model
    for (const auto& sid : impact.deletedStatementIds) {
        const QString qid = QString::fromStdString(sid);
        // find and remove from statements_
        auto stmts = statements_.statements();
        for (int i = 0; i < static_cast<int>(stmts.size()); ++i) {
            const auto& s = stmts[static_cast<size_t>(i)];
            if (!s) continue;
            if (QString::fromStdString(s->id) == qid) {
                statements_.removeAt(i);
                if (selectedStatementId_ == qid) { selectedStatementId_.clear(); emit selectedStatementIdChanged(); selectedStatement_.clear(); }
                break;
            }
        }
    }

    // Remove deleted properties and actors similarly (update lists and selections)
    for (const auto& pid : impact.deletedPropertyIds) {
        const QString qid = QString::fromStdString(pid);
        auto props = properties_.properties();
        for (int i = 0; i < static_cast<int>(props.size()); ++i) {
            const auto& p = props[static_cast<size_t>(i)];
            if (!p) continue;
            if (QString::fromStdString(p->id) == qid) { properties_.removeAt(i); if (selectedPropertyId_ == qid) { selectedPropertyId_.clear(); emit selectedPropertyIdChanged(); selectedProperty_.clear(); } break; }
        }
        // Also remove property id references from transaction selection and internal state lists
    }

    for (const auto& aid : impact.deletedActorIds) {
        const QString qid = QString::fromStdString(aid);
        auto acts = actors_.actors();
        for (int i = 0; i < static_cast<int>(acts.size()); ++i) {
            const auto& a = acts[static_cast<size_t>(i)];
            if (!a) continue;
            if (QString::fromStdString(a->id) == qid) { actors_.removeAt(i); if (selectedActorId_ == qid) { selectedActorId_.clear(); emit selectedActorIdChanged(); selectedActor_.clear(); } break; }
        }
    }

    // Contracts: remove from list
    for (const auto& cid : impact.deletedContractIds) {
        const QString qid = QString::fromStdString(cid);
        auto cs = contracts_.contracts();
        for (int i = 0; i < static_cast<int>(cs.size()); ++i) {
            const auto& c = cs[static_cast<size_t>(i)];
            if (!c) continue;
            if (QString::fromStdString(c->id) == qid) { contracts_.removeAt(i); break; }
        }
    }
}

// Called by QML when editing state changes; if editing stopped, apply any pending deletions
void UiDataSession::setEditingTransaction(const QString& txId, bool editing)
{
    if (editing) editingTransactions_.insert(txId);
    else {
        editingTransactions_.remove(txId);
        if (pendingDeletionTransactions_.contains(txId)) {
            // apply pending deletion now
            int row = transactions_.findRowById(txId);
            if (row >= 0) transactions_.removeAt(row);
            if (selectedTransactionId_ == txId) {
                selectedTransactionId_.clear();
                emit selectedTransactionIdChanged();
                selectedTransaction_.clear();
            }
            pendingDeletionTransactions_.remove(txId);
        }
    }
}

bool UiDataSession::isEditingTransaction(const QString& txId) const
{
    return editingTransactions_.contains(txId);
}
