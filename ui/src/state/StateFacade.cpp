#include "ui/state/StateFacade.h"

#include <algorithm>
#include <QAbstractItemModel>
#include <QSet>

namespace ui {

void StateFacade::rebuildPropertyNameIndex()
{
    propertyNameIndex_.clear();
    const auto& rows = properties_.properties();
    propertyNameIndex_.reserve(static_cast<int>(rows.size()));
    for (const auto& p : rows) {
        if (!p) continue;
        propertyNameIndex_.insert(QString::fromStdString(p->id), QString::fromStdString(p->name));
    }
}

void StateFacade::recomputeAllMetrics()
{
    metrics_.recomputeAll(properties_, transactions_, contracts_, [this](const QString& propertyId) {
        emit transactionSumsUpdated(propertyId);
    });
}

void StateFacade::recomputeMetricsForPropertyIds(const QSet<QString>& propertyIds)
{
    for (const auto& propertyId : propertyIds) {
        metrics_.recomputeProperty(propertyId, transactions_, contracts_, [this](const QString& pid) {
            emit transactionSumsUpdated(pid);
        });
    }
}

void StateFacade::recomputeMetricsForRows(int firstRow, int lastRow)
{
    if (firstRow < 0 || lastRow < firstRow) {
        recomputeAllMetrics();
        return;
    }

    QSet<QString> affectedPropertyIds;
    const auto rows = transactions_.transactions();
    const int safeLast = std::min(lastRow, static_cast<int>(rows.size()) - 1);
    for (int row = firstRow; row <= safeLast; ++row) {
        const auto& tx = rows[static_cast<size_t>(row)];
        if (!tx) continue;
        for (const auto& pid : tx->propertyIds) affectedPropertyIds.insert(QString::fromStdString(pid));
    }

    if (affectedPropertyIds.isEmpty()) {
        recomputeAllMetrics();
        return;
    }

    recomputeMetricsForPropertyIds(affectedPropertyIds);
}

void StateFacade::notifyTransactionSumsForAllProperties()
{
    for (const auto& p : properties_.properties()) {
        if (!p) continue;
        emit transactionSumsUpdated(QString::fromStdString(p->id));
    }
}

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
        recomputeAllMetrics();
    });

    connect(&transactions_, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex&, int first, int last) {
        recomputeMetricsForRows(first, last);
    });

    connect(&transactions_, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
        if (roles.isEmpty()) {
            recomputeAllMetrics();
            return;
        }

        bool propertyIdsChanged = false;
        bool amountOrAllocChanged = false;
        for (int role : roles) {
            if (role == TransactionList::PropertyIdsRole) propertyIdsChanged = true;
            if (role == TransactionList::AmountRole || role == TransactionList::AllocatableRole) amountOrAllocChanged = true;
        }

        if (propertyIdsChanged) {
            recomputeAllMetrics();
            return;
        }

        if (!amountOrAllocChanged) return;

        const int first = topLeft.isValid() ? topLeft.row() : -1;
        const int last = bottomRight.isValid() ? bottomRight.row() : first;
        recomputeMetricsForRows(first, last);
    });

    connect(&transactions_, &QAbstractItemModel::modelReset, this, [this]() {
        recomputeAllMetrics();
    });

    connect(&analyses_, &QAbstractItemModel::modelReset, this, [this]() {
        metrics_.clearCache();
        notifyTransactionSumsForAllProperties();
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
    rebuildPropertyNameIndex();

    recomputeAllMetrics();

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

ActorSelection* StateFacade::selectedActor() { return selection_.selectedActor(); }
PropertySelection* StateFacade::selectedProperty() { return selection_.selectedProperty(); }
ContractSelection* StateFacade::selectedContract() { return selection_.selectedContract(); }
StatementSelection* StateFacade::selectedStatement() { return selection_.selectedStatement(); }
TransactionSelection* StateFacade::selectedTransaction() { return selection_.selectedTransaction(); }
AnalysisSelection* StateFacade::selectedAnalysis() { return selection_.selectedAnalysis(); }
AnnualSelection* StateFacade::selectedAnnual() { return selection_.selectedAnnual(); }

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
    const auto it = propertyNameIndex_.find(id);
    return it == propertyNameIndex_.end() ? id : it.value();
}

void StateFacade::applyDeletionImpact(const DeletionImpact& impact)
{
    for (const auto& tid : impact.deletedTransactionIds) {
        const QString qid = QString::fromStdString(tid);
        int row = transactions_.findRowById(qid);
        if (row >= 0) {
            const auto tptr = transactions_.transactions().at(static_cast<size_t>(row));
            QSet<QString> affected;
            if (tptr) {
                for (const auto& pid : tptr->propertyIds) affected.insert(QString::fromStdString(pid));
            }
            transactions_.removeAt(row);
            recomputeMetricsForPropertyIds(affected);
        }
        if (selection_.clearTransactionIfSelected(qid)) emit selectedTransactionIdChanged();
    }

    for (const auto& sid : impact.deletedStatementIds) {
        const QString qid = QString::fromStdString(sid);
        const int row = statements_.findRowById(qid);
        if (row >= 0) statements_.removeAt(row);
        filters_.removeStatement(qid);
        if (selection_.clearStatementIfSelected(qid)) emit selectedStatementIdChanged();
    }

    for (const auto& pid : impact.deletedPropertyIds) {
        const QString qid = QString::fromStdString(pid);
        const int row = properties_.findRowById(qid);
        if (row >= 0) properties_.removeAt(row);
        propertyNameIndex_.remove(qid);
        metrics_.removePropertyCache(qid);
        filters_.removeProperty(qid);
        if (selection_.clearPropertyIfSelected(qid)) emit selectedPropertyIdChanged();
    }

    for (const auto& aid : impact.deletedActorIds) {
        const QString qid = QString::fromStdString(aid);
        const int row = actors_.findRowById(qid);
        if (row >= 0) actors_.removeAt(row);
        if (selection_.clearActorIfSelected(qid)) emit selectedActorIdChanged();
    }

    for (const auto& cid : impact.deletedContractIds) {
        const QString qid = QString::fromStdString(cid);
        const int row = contracts_.findRowById(qid);
        if (row >= 0) contracts_.removeAt(row);
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
    recomputeMetricsForPropertyIds(affected);
}

}
