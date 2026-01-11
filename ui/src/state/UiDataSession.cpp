#include "ui/state/UiDataSession.h"
#include <QDebug>
#include <QAbstractItemModel>
#include "ui/models/TransactionList.h"

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
    // Connect to transactions_ model signals to keep property sums cache up-to-date
    connect(&transactions_, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex& parent, int first, int last){
        Q_UNUSED(parent);
        const auto vec = transactions_.transactions();
        for (int r = first; r <= last; ++r) {
            if (r < 0 || r >= static_cast<int>(vec.size())) continue;
            const auto& t = vec[static_cast<size_t>(r)];
            if (!t) continue;
            for (const auto& pid : t->propertyIds) pendingRecomputePropertyIds_.insert(QString::fromStdString(pid));
        }
    });

    connect(&transactions_, &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex& parent, int first, int last){
        Q_UNUSED(parent);
        // recompute for all property ids captured in aboutToBeRemoved
        for (const auto& pid : pendingRecomputePropertyIds_) recomputePropertySum(pid);
        pendingRecomputePropertyIds_.clear();
    });

    connect(&transactions_, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent, int first, int last){
        Q_UNUSED(parent);
        const auto vec = transactions_.transactions();
        for (int r = first; r <= last; ++r) {
            if (r < 0 || r >= static_cast<int>(vec.size())) continue;
            const auto& t = vec[static_cast<size_t>(r)];
            if (!t) continue;
            for (const auto& pid : t->propertyIds) recomputePropertySum(QString::fromStdString(pid));
        }
    });

    connect(&transactions_, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles){
        // Only react when roles affecting sums changed; if roles is empty treat as full update
        if (!roles.isEmpty()) {
            bool relevant = false;
            for (int r : roles) {
                if (r == TransactionList::AmountRole || r == TransactionList::AllocatableRole || r == TransactionList::PropertyIdsRole) { relevant = true; break; }
            }
            if (!relevant) return;
        }

        const auto vec = transactions_.transactions();
        for (int r = topLeft.row(); r <= bottomRight.row(); ++r) {
            if (r < 0 || r >= static_cast<int>(vec.size())) continue;
            const auto& t = vec[static_cast<size_t>(r)];
            if (!t) continue;
            for (const auto& pid : t->propertyIds) recomputePropertySum(QString::fromStdString(pid));
        }
    });

    connect(&transactions_, &QAbstractItemModel::modelReset, this, [this](){ recomputeAllPropertySums(); });
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

    // recompute sums cache after loading
    recomputeAllPropertySums();

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

QObject* UiDataSession::transactionsForProperty(const QString& propertyId)
{
    if (propertyId.isEmpty()) return nullptr;

    const auto key = propertyId;
    if (txByProperty_.contains(key)) return txByProperty_.value(key);

    auto* proxy = new TransactionFilterModel(this);
    proxy->setSourceModel(&transactions_);
    proxy->setPropertyId(propertyId);
    txByProperty_.insert(key, proxy);
    return proxy;
}

QVariantMap UiDataSession::transactionSumsForProperty(const QString& propertyId) const
{
    if (propertyId.isEmpty()) return {};
    if (propertySumsCache_.contains(propertyId)) return propertySumsCache_.value(propertyId);

    // compute on demand and cache
    QVariantMap out;
    out["total"] = 0.0;
    out["allocatable"] = 0.0;
    out["nonAllocatable"] = 0.0;

    for (const auto& t : transactions_.transactions()) {
        if (!t) continue;
        for (const auto& pid : t->propertyIds) {
            if (QString::fromStdString(pid) == propertyId) {
                out["total"] = out["total"].toDouble() + t->amount;
                if (t->allocatable) out["allocatable"] = out["allocatable"].toDouble() + t->amount;
                else out["nonAllocatable"] = out["nonAllocatable"].toDouble() + t->amount;
                break;
            }
        }
    }

    propertySumsCache_.insert(propertyId, out);
    return out;
}

void UiDataSession::recomputeAllPropertySums()
{
    propertySumsCache_.clear();
    // build set of property ids from properties_ model
    for (const auto& p : properties_.properties()) {
        if (!p) continue;
        recomputePropertySum(QString::fromStdString(p->id));
    }
}

void UiDataSession::recomputePropertySum(const QString& propertyId) const
{
    if (propertyId.isEmpty()) return;
    QVariantMap out;
    out["total"] = 0.0;
    out["allocatable"] = 0.0;
    out["nonAllocatable"] = 0.0;

    for (const auto& t : transactions_.transactions()) {
        if (!t) continue;
        for (const auto& pid : t->propertyIds) {
            if (QString::fromStdString(pid) == propertyId) {
                out["total"] = out["total"].toDouble() + t->amount;
                if (t->allocatable) out["allocatable"] = out["allocatable"].toDouble() + t->amount;
                else out["nonAllocatable"] = out["nonAllocatable"].toDouble() + t->amount;
                break;
            }
        }
    }

    propertySumsCache_.insert(propertyId, out);
    // emit change to QML listeners
    QMetaObject::invokeMethod(const_cast<UiDataSession*>(this), "transactionSumsUpdated", Qt::QueuedConnection, Q_ARG(QString, propertyId));
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

                    // Invalidate sums for affected properties: union of old and new propertyIds
                    QSet<QString> affected;
                    if (current) for (const auto& pid : current->propertyIds) affected.insert(QString::fromStdString(pid));
                    for (const auto& pid : t->propertyIds) affected.insert(QString::fromStdString(pid));
                    for (const auto& pid : affected) {
                        // recompute and notify
                        recomputePropertySum(pid);
                    }
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
        if (row >= 0) {
            // collect affected property ids from transaction before removal
            const auto tptr = transactions_.transactions().at(static_cast<size_t>(row));
            QStringList affected;
            if (tptr) for (const auto& pid : tptr->propertyIds) affected << QString::fromStdString(pid);

            transactions_.removeAt(row);

            for (const auto& pid : affected) recomputePropertySum(pid);
        }
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
        propertySumsCache_.remove(qid);
        txByProperty_.remove(qid);
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

void UiDataSession::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    if (txId.isEmpty()) return;
    int row = transactions_.findRowById(txId);
    if (row < 0) return;

    auto current = transactions_.transactions().at(static_cast<size_t>(row));
    if (!current) return;

    // compute old and new sets
    QSet<QString> oldSet;
    for (const auto& pid : current->propertyIds) oldSet.insert(QString::fromStdString(pid));

    QSet<QString> newSet;
    for (const auto& pid : propertyIds) newSet.insert(pid);

    if (oldSet == newSet) return;

    // apply to model object
    current->propertyIds.clear();
    for (const auto& pid : propertyIds) current->propertyIds.push_back(pid.toStdString());

    // emit dataChanged for propertyIds role
    const QModelIndex mi = transactions_.index(row);
    QVector<int> roles; roles.append(TransactionList::PropertyIdsRole);
    emit transactions_.dataChanged(mi, mi, roles);

    // recompute sums for affected properties (union)
    QSet<QString> affected = oldSet;
    affected.unite(newSet);
    for (const auto& pid : affected) recomputePropertySum(pid);
}
