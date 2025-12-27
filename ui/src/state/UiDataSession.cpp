#include "ui/state/UiDataSession.h"

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
                                            QString::fromStdString(s->name),
                                            QString::fromStdString(s->startDate),
                                            QString::fromStdString(s->endDate));
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
            selectedTransaction_.setTransaction(QString::fromStdString(t->id),
                                                QString::fromStdString(t->name),
                                                QString::fromStdString(t->bookingDate),
                                                t->amount,
                                                QString::fromStdString(t->description),
                                                QString::fromStdString(t->statementId));
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
