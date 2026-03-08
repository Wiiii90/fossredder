#include "ui/state/SessionStore.h"

#include <algorithm>

#include <QAbstractItemModel>

namespace ui {

SessionStore::SessionStore(QObject* owner, TransactionSumsNotifier notifier)
    : owner_(owner)
    , transactionSumsNotifier_(std::move(notifier))
    , actors_(owner)
    , analyses_(owner)
    , properties_(owner)
    , contracts_(owner)
    , statements_(owner)
    , transactions_(owner)
    , annuals_(owner)
    , selection_(actors_, properties_, contracts_, statements_, transactions_, analyses_, annuals_, owner)
{
    bindModelSignals();
}

void SessionStore::bindModelSignals()
{
    QObject::connect(&transactions_, &QAbstractItemModel::rowsRemoved, &transactions_, [this](const QModelIndex&, int, int) {
        recomputeAllMetrics();
    });

    QObject::connect(&transactions_, &QAbstractItemModel::rowsInserted, &transactions_, [this](const QModelIndex&, int first, int last) {
        recomputeMetricsForRows(first, last);
    });

    QObject::connect(&transactions_, &QAbstractItemModel::dataChanged, &transactions_, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
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

    QObject::connect(&transactions_, &QAbstractItemModel::modelReset, &transactions_, [this]() {
        recomputeAllMetrics();
    });

    QObject::connect(&analyses_, &QAbstractItemModel::modelReset, &analyses_, [this]() {
        metrics_.clearCache();
        notifyTransactionSumsForAllProperties();
    });
}

void SessionStore::rebuildPropertyNameIndex()
{
    propertyNameIndex_.clear();
    const auto& rows = properties_.properties();
    propertyNameIndex_.reserve(static_cast<int>(rows.size()));
    for (const auto& property : rows) {
        if (!property) continue;
        propertyNameIndex_.insert(QString::fromStdString(property->id), QString::fromStdString(property->name));
    }
}

void SessionStore::recomputeAllMetrics()
{
    metrics_.recomputeAll(properties_, transactions_, contracts_, [this](const QString& propertyId) {
        if (transactionSumsNotifier_) transactionSumsNotifier_(propertyId);
    });
}

void SessionStore::recomputeMetricsForPropertyIds(const QSet<QString>& propertyIds)
{
    for (const auto& propertyId : propertyIds) {
        metrics_.recomputeProperty(propertyId, transactions_, contracts_, [this](const QString& id) {
            if (transactionSumsNotifier_) transactionSumsNotifier_(id);
        });
    }
}

void SessionStore::recomputeMetricsForRows(int firstRow, int lastRow)
{
    if (firstRow < 0 || lastRow < firstRow) {
        recomputeAllMetrics();
        return;
    }

    QSet<QString> affectedPropertyIds;
    const auto rows = transactions_.transactions();
    const int safeLast = std::min(lastRow, static_cast<int>(rows.size()) - 1);
    for (int row = firstRow; row <= safeLast; ++row) {
        const auto& transaction = rows[static_cast<size_t>(row)];
        if (!transaction) continue;
        for (const auto& propertyId : transaction->propertyIds) affectedPropertyIds.insert(QString::fromStdString(propertyId));
    }

    if (affectedPropertyIds.isEmpty()) {
        recomputeAllMetrics();
        return;
    }

    recomputeMetricsForPropertyIds(affectedPropertyIds);
}

void SessionStore::notifyTransactionSumsForAllProperties() const
{
    if (!transactionSumsNotifier_) return;

    for (const auto& property : properties_.properties()) {
        if (!property) continue;
        transactionSumsNotifier_(QString::fromStdString(property->id));
    }
}

void SessionStore::loadFromState(const AppState& state)
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
}

QString SessionStore::selectedActorId() const { return selection_.selectedActorId(); }
QString SessionStore::selectedPropertyId() const { return selection_.selectedPropertyId(); }
QString SessionStore::selectedContractId() const { return selection_.selectedContractId(); }
QString SessionStore::selectedStatementId() const { return selection_.selectedStatementId(); }
QString SessionStore::selectedTransactionId() const { return selection_.selectedTransactionId(); }
QString SessionStore::selectedAnalysisId() const { return selection_.selectedAnalysisId(); }
QString SessionStore::selectedAnnualId() const { return selection_.selectedAnnualId(); }

bool SessionStore::setSelectedActorId(const QString& id) { return selection_.setSelectedActorId(id); }
bool SessionStore::setSelectedPropertyId(const QString& id) { return selection_.setSelectedPropertyId(id); }
bool SessionStore::setSelectedContractId(const QString& id) { return selection_.setSelectedContractId(id); }
bool SessionStore::setSelectedStatementId(const QString& id) { return selection_.setSelectedStatementId(id); }
bool SessionStore::setSelectedTransactionId(const QString& id) { return selection_.setSelectedTransactionId(id); }
bool SessionStore::setSelectedAnalysisId(const QString& id) { return selection_.setSelectedAnalysisId(id); }
bool SessionStore::setSelectedAnnualId(const QString& id) { return selection_.setSelectedAnnualId(id); }

ActorSelection* SessionStore::selectedActor() { return selection_.selectedActor(); }
PropertySelection* SessionStore::selectedProperty() { return selection_.selectedProperty(); }
ContractSelection* SessionStore::selectedContract() { return selection_.selectedContract(); }
StatementSelection* SessionStore::selectedStatement() { return selection_.selectedStatement(); }
TransactionSelection* SessionStore::selectedTransaction() { return selection_.selectedTransaction(); }
AnalysisSelection* SessionStore::selectedAnalysis() { return selection_.selectedAnalysis(); }
AnnualSelection* SessionStore::selectedAnnual() { return selection_.selectedAnnual(); }

QVariantList SessionStore::statementTransactionIds(const QString& statementId) const
{
    QVariantList out;
    if (statementId.isEmpty()) return out;
    for (const auto& transaction : transactions_.transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) == statementId) out.push_back(QString::fromStdString(transaction->id));
    }
    return out;
}

QObject* SessionStore::statementTransactions(const QString& statementId)
{
    return filters_.statementTransactions(statementId, transactions_, owner_);
}

QObject* SessionStore::propertyTransactions(const QString& propertyId)
{
    return filters_.propertyTransactions(propertyId, transactions_, owner_);
}

QStringList SessionStore::propertyContractTypes(const QString& propertyId) const
{
    return metrics_.propertyContractTypes(propertyId, transactions_, contracts_);
}

QVariantMap SessionStore::propertyTransactionSums(const QString& propertyId, const QString& contractType) const
{
    return metrics_.propertyTransactionSums(propertyId, contractType, transactions_, contracts_);
}

QString SessionStore::propertyName(const QString& id) const
{
    if (id.isEmpty()) return {};
    const auto it = propertyNameIndex_.find(id);
    return it == propertyNameIndex_.end() ? id : it.value();
}

void SessionStore::applyDeletionImpact(const DeletionImpact& impact)
{
    for (const auto& transactionId : impact.deletedTransactionIds) {
        const QString id = QString::fromStdString(transactionId);
        const int row = transactions_.findRowById(id);
        if (row >= 0) {
            const auto transaction = transactions_.transactions().at(static_cast<size_t>(row));
            QSet<QString> affectedPropertyIds;
            if (transaction) {
                for (const auto& propertyId : transaction->propertyIds) affectedPropertyIds.insert(QString::fromStdString(propertyId));
            }
            transactions_.removeAt(row);
            recomputeMetricsForPropertyIds(affectedPropertyIds);
        }
        selection_.clearTransactionIfSelected(id);
    }

    for (const auto& statementId : impact.deletedStatementIds) {
        const QString id = QString::fromStdString(statementId);
        const int row = statements_.findRowById(id);
        if (row >= 0) statements_.removeAt(row);
        filters_.removeStatement(id);
        selection_.clearStatementIfSelected(id);
    }

    for (const auto& propertyId : impact.deletedPropertyIds) {
        const QString id = QString::fromStdString(propertyId);
        const int row = properties_.findRowById(id);
        if (row >= 0) properties_.removeAt(row);
        propertyNameIndex_.remove(id);
        metrics_.removePropertyCache(id);
        filters_.removeProperty(id);
        selection_.clearPropertyIfSelected(id);
    }

    for (const auto& actorId : impact.deletedActorIds) {
        const QString id = QString::fromStdString(actorId);
        const int row = actors_.findRowById(id);
        if (row >= 0) actors_.removeAt(row);
        selection_.clearActorIfSelected(id);
    }

    for (const auto& contractId : impact.deletedContractIds) {
        const QString id = QString::fromStdString(contractId);
        const int row = contracts_.findRowById(id);
        if (row >= 0) contracts_.removeAt(row);
        selection_.clearContractIfSelected(id);
    }
}

void SessionStore::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    if (txId.isEmpty()) return;
    const int row = transactions_.findRowById(txId);
    if (row < 0) return;

    auto current = transactions_.transactions().at(static_cast<size_t>(row));
    if (!current) return;

    QSet<QString> oldSet;
    for (const auto& propertyId : current->propertyIds) oldSet.insert(QString::fromStdString(propertyId));

    QSet<QString> newSet;
    for (const auto& propertyId : propertyIds) newSet.insert(propertyId);

    if (oldSet == newSet) return;

    current->propertyIds.clear();
    for (const auto& propertyId : propertyIds) current->propertyIds.push_back(propertyId.toStdString());

    const QModelIndex modelIndex = transactions_.index(row);
    emit transactions_.dataChanged(modelIndex, modelIndex, {TransactionList::PropertyIdsRole});

    oldSet.unite(newSet);
    recomputeMetricsForPropertyIds(oldSet);
}

}
