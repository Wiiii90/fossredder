#include "ui/state/StateFacade.h"

#include <QAbstractItemModel>
#include <QSet>
#include <QMetaObject>

namespace ui {

StateFacade::StateFacade(QObject* parent)
    : QObject(parent)
    , actors_(this)
    , analyses_(this)
    , properties_(this)
    , contracts_(this)
    , statements_(this)
    , transactions_(this)
    , annuals_(this)
    , selection_(actors_, properties_, contracts_, statements_, transactions_, analyses_, annuals_, this)
{
    connect(&transactions_, &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex&, int, int) {
        metrics_.recomputeAll(properties_, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });
    });

    connect(&transactions_, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex&, int, int) {
        metrics_.recomputeAll(properties_, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });
    });

    connect(&transactions_, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex&, const QModelIndex&, const QVector<int>& roles) {
        if (!roles.isEmpty()) {
            bool relevant = false;
            for (int role : roles) {
                if (role == TransactionList::AmountRole || role == TransactionList::AllocatableRole || role == TransactionList::PropertyIdsRole) {
                    relevant = true;
                    break;
                }
            }
            if (!relevant) return;
        }
        metrics_.recomputeAll(properties_, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });
    });

    connect(&transactions_, &QAbstractItemModel::modelReset, this, [this]() {
        metrics_.recomputeAll(properties_, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });
    });

    connect(&analyses_, &QAbstractItemModel::modelReset, this, [this]() {
        metrics_.clearCache();
        for (const auto& p : properties_.properties()) {
            if (!p) continue;
            emit transactionSumsUpdated(QString::fromStdString(p->id));
        }
    });
}

void StateFacade::loadFromState(const AppState& state)
{
    filters_.clear();

    actors_.setActors(state.actors);
    properties_.setProperties(state.properties);
    contracts_.setContracts(state.contracts);
    statements_.setStatements(state.statements);
    transactions_.setTransactions(state.transactions);
    analyses_.setAnalyses(state.analyses);
    annuals_.setAnnuals(state.annuals);
    transactions_.setContracts(state.contracts);

    metrics_.recomputeAll(properties_, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });

    selection_.refreshAll();
    lastAnalysisResult_.clear();
}

QString StateFacade::selectedActorId() const { return selection_.selectedActorId(); }
QString StateFacade::selectedPropertyId() const { return selection_.selectedPropertyId(); }
QString StateFacade::selectedContractId() const { return selection_.selectedContractId(); }
QString StateFacade::selectedStatementId() const { return selection_.selectedStatementId(); }
QString StateFacade::selectedTransactionId() const { return selection_.selectedTransactionId(); }
QString StateFacade::selectedAnalysisId() const { return selection_.selectedAnalysisId(); }
QString StateFacade::selectedAnnualId() const { return selection_.selectedAnnualId(); }

void StateFacade::setSelectedActorId(const QString& id)
{
    if (selection_.setSelectedActorId(id)) emit selectedActorIdChanged();
}

void StateFacade::setSelectedPropertyId(const QString& id)
{
    if (selection_.setSelectedPropertyId(id)) emit selectedPropertyIdChanged();
}

void StateFacade::setSelectedContractId(const QString& id)
{
    if (selection_.setSelectedContractId(id)) emit selectedContractIdChanged();
}

void StateFacade::setSelectedStatementId(const QString& id)
{
    if (selection_.setSelectedStatementId(id)) emit selectedStatementIdChanged();
}

void StateFacade::setSelectedTransactionId(const QString& id)
{
    if (selection_.setSelectedTransactionId(id)) emit selectedTransactionIdChanged();
}

void StateFacade::setSelectedAnalysisId(const QString& id)
{
    if (selection_.setSelectedAnalysisId(id)) emit selectedAnalysisIdChanged();
}

void StateFacade::setSelectedAnnualId(const QString& id)
{
    if (selection_.setSelectedAnnualId(id)) emit selectedAnnualIdChanged();
}

EntitySelection* StateFacade::selectedActor() { return selection_.selectedActor(); }
EntitySelection* StateFacade::selectedProperty() { return selection_.selectedProperty(); }
EntitySelection* StateFacade::selectedContract() { return selection_.selectedContract(); }
EntitySelection* StateFacade::selectedStatement() { return selection_.selectedStatement(); }
EntitySelection* StateFacade::selectedTransaction() { return selection_.selectedTransaction(); }
EntitySelection* StateFacade::selectedAnalysis() { return selection_.selectedAnalysis(); }
EntitySelection* StateFacade::selectedAnnual() { return selection_.selectedAnnual(); }

QVariantList StateFacade::statementTransactionIds(const QString& statementId) const
{
    QVariantList out;
    if (statementId.isEmpty()) return out;
    for (const auto& t : transactions_.transactions()) {
        if (!t) continue;
        if (QString::fromStdString(t->statementId) == statementId) out.push_back(QString::fromStdString(t->id));
    }
    return out;
}

QObject* StateFacade::statementTransactions(const QString& statementId)
{
    return filters_.statementTransactions(statementId, transactions_, this);
}

QObject* StateFacade::propertyTransactions(const QString& propertyId)
{
    return filters_.propertyTransactions(propertyId, transactions_, this);
}

QStringList StateFacade::propertyContractTypes(const QString& propertyId) const
{
    return metrics_.propertyContractTypes(propertyId, transactions_, contracts_);
}

QVariantMap StateFacade::propertyTransactionSums(const QString& propertyId, const QString& contractType) const
{
    return metrics_.propertyTransactionSums(propertyId, contractType, transactions_, contracts_);
}

QString StateFacade::propertyName(const QString& id) const
{
    if (id.isEmpty()) return QString();
    for (const auto& p : properties_.properties()) {
        if (!p) continue;
        if (QString::fromStdString(p->id) == id) return QString::fromStdString(p->name);
    }
    return id;
}

void StateFacade::applyTransactionUpdates(const std::vector<std::string>& ids, const AppState& state)
{
    if (ids.empty()) return;
    for (const auto& sid : ids) {
        const QString qid = QString::fromStdString(sid);
        int row = transactions_.findRowById(qid);
        if (row < 0) continue;

        for (const auto& updated : state.transactions) {
            if (!updated) continue;
            if (QString::fromStdString(updated->id) != qid) continue;

            const auto current = transactions_.transactions().at(static_cast<size_t>(row));
            bool changed = !current;
            if (current) {
                if (current->allocatable != updated->allocatable) changed = true;
                else if (current->status != updated->status) changed = true;
                else if (current->amount != updated->amount) changed = true;
                else if (current->name != updated->name) changed = true;
                else if (current->bookingDate != updated->bookingDate) changed = true;
                else if (current->description != updated->description) changed = true;
                else if (current->propertyIds != updated->propertyIds) changed = true;
            }

            if (!changed) break;

            transactions_.setTransactionAt(row, updated);
            if (selection_.selectedTransactionId() == qid) selection_.refreshSelectedTransaction();

            QSet<QString> affected;
            if (current) for (const auto& pid : current->propertyIds) affected.insert(QString::fromStdString(pid));
            for (const auto& pid : updated->propertyIds) affected.insert(QString::fromStdString(pid));
            for (const auto& pid : affected) {
                metrics_.recomputeProperty(pid, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });
            }
            break;
        }
    }
}

void StateFacade::applyDeletionImpact(const DeletionImpact& impact)
{
    auto removeById = [](const auto& items, const QString& id, auto removeAt) {
        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            const auto& item = items[static_cast<size_t>(i)];
            if (!item) continue;
            if (QString::fromStdString(item->id) != id) continue;
            removeAt(i);
            return true;
        }
        return false;
    };

    for (const auto& tid : impact.deletedTransactionIds) {
        const QString qid = QString::fromStdString(tid);
        int row = transactions_.findRowById(qid);
        if (row >= 0) {
            const auto tptr = transactions_.transactions().at(static_cast<size_t>(row));
            QStringList affected;
            if (tptr) for (const auto& pid : tptr->propertyIds) affected << QString::fromStdString(pid);
            transactions_.removeAt(row);
            for (const auto& pid : affected) {
                metrics_.recomputeProperty(pid, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });
            }
        }
        if (selection_.clearTransactionIfSelected(qid)) emit selectedTransactionIdChanged();
    }

    for (const auto& sid : impact.deletedStatementIds) {
        const QString qid = QString::fromStdString(sid);
        removeById(statements_.statements(), qid, [&](int rowIndex) { statements_.removeAt(rowIndex); });
        filters_.removeStatement(qid);
        if (selection_.clearStatementIfSelected(qid)) emit selectedStatementIdChanged();
    }

    for (const auto& pid : impact.deletedPropertyIds) {
        const QString qid = QString::fromStdString(pid);
        removeById(properties_.properties(), qid, [&](int rowIndex) { properties_.removeAt(rowIndex); });
        metrics_.removePropertyCache(qid);
        filters_.removeProperty(qid);
        if (selection_.clearPropertyIfSelected(qid)) emit selectedPropertyIdChanged();
    }

    for (const auto& aid : impact.deletedActorIds) {
        const QString qid = QString::fromStdString(aid);
        removeById(actors_.actors(), qid, [&](int rowIndex) { actors_.removeAt(rowIndex); });
        if (selection_.clearActorIfSelected(qid)) emit selectedActorIdChanged();
    }

    for (const auto& cid : impact.deletedContractIds) {
        const QString qid = QString::fromStdString(cid);
        removeById(contracts_.contracts(), qid, [&](int rowIndex) { contracts_.removeAt(rowIndex); });
        if (selection_.clearContractIfSelected(qid)) emit selectedContractIdChanged();
    }
}

void StateFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    if (txId.isEmpty()) return;
    int row = transactions_.findRowById(txId);
    if (row < 0) return;

    auto current = transactions_.transactions().at(static_cast<size_t>(row));
    if (!current) return;

    QSet<QString> oldSet;
    for (const auto& pid : current->propertyIds) oldSet.insert(QString::fromStdString(pid));

    QSet<QString> newSet;
    for (const auto& pid : propertyIds) newSet.insert(pid);

    if (oldSet == newSet) return;

    current->propertyIds.clear();
    for (const auto& pid : propertyIds) current->propertyIds.push_back(pid.toStdString());

    const QModelIndex modelIndex = transactions_.index(row);
    QVector<int> roles;
    roles.append(TransactionList::PropertyIdsRole);
    emit transactions_.dataChanged(modelIndex, modelIndex, roles);

    QSet<QString> affected = oldSet;
    affected.unite(newSet);
    for (const auto& pid : affected) {
        metrics_.recomputeProperty(pid, transactions_, contracts_, [this](const QString& propertyId) { emit transactionSumsUpdated(propertyId); });
    }
}

}
