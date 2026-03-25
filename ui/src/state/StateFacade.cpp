/**
 * @file ui/src/state/StateFacade.cpp
 * @brief Implements the UI session facade that combines session data and selection state.
 */

#include "ui/state/StateFacade.h"

namespace ui {

StateFacade::StateFacade(QObject* parent)
    : QObject(parent)
    , session_(std::make_unique<SessionStore>(this))
    , selection_(std::make_unique<SessionSelection>(session_->models(), this))
{
    QObject::connect(session_.get(), &SessionStore::transactionSumsUpdated, this, &StateFacade::transactionSumsUpdated);
    QObject::connect(selection_.get(), &SessionSelection::selectedActorIdChanged, this, &StateFacade::selectedActorIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedPropertyIdChanged, this, &StateFacade::selectedPropertyIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedContractIdChanged, this, &StateFacade::selectedContractIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedStatementIdChanged, this, &StateFacade::selectedStatementIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedTransactionIdChanged, this, &StateFacade::selectedTransactionIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedAnalysisIdChanged, this, &StateFacade::selectedAnalysisIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedAnnualIdChanged, this, &StateFacade::selectedAnnualIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::lastAnalysisResultChanged, this, &StateFacade::lastAnalysisResultChanged);
}

SessionStore* StateFacade::session() noexcept { return session_.get(); }
SessionSelection* StateFacade::selection() noexcept { return selection_.get(); }

void StateFacade::loadFromState(const AppState& state)
{
    session_->loadFromState(state);
    selection_->loadFromState();
}

ActorList* StateFacade::actors() noexcept { return &session_->models().actors(); }
PropertyList* StateFacade::properties() noexcept { return &session_->models().properties(); }
ContractList* StateFacade::contracts() noexcept { return &session_->models().contracts(); }
StatementList* StateFacade::statements() noexcept { return &session_->models().statements(); }
TransactionList* StateFacade::transactions() noexcept { return &session_->models().transactions(); }
AnalysisList* StateFacade::analyses() noexcept { return &session_->models().analyses(); }
AnnualList* StateFacade::annuals() noexcept { return &session_->models().annuals(); }

QString StateFacade::selectedActorId() const { return selection_->selectedActorId(); }
QString StateFacade::selectedPropertyId() const { return selection_->selectedPropertyId(); }
QString StateFacade::selectedContractId() const { return selection_->selectedContractId(); }
QString StateFacade::selectedStatementId() const { return selection_->selectedStatementId(); }
QString StateFacade::selectedTransactionId() const { return selection_->selectedTransactionId(); }
QString StateFacade::selectedAnalysisId() const { return selection_->selectedAnalysisId(); }
QString StateFacade::selectedAnnualId() const { return selection_->selectedAnnualId(); }

void StateFacade::setSelectedActorId(const QString& id)
{
    selection_->setSelectedActorId(id);
}

void StateFacade::setSelectedPropertyId(const QString& id)
{
    selection_->setSelectedPropertyId(id);
}

void StateFacade::setSelectedContractId(const QString& id)
{
    selection_->setSelectedContractId(id);
}

void StateFacade::setSelectedStatementId(const QString& id)
{
    selection_->setSelectedStatementId(id);
}

void StateFacade::setSelectedTransactionId(const QString& id)
{
    selection_->setSelectedTransactionId(id);
}

void StateFacade::setSelectedAnalysisId(const QString& id)
{
    selection_->setSelectedAnalysisId(id);
}

void StateFacade::setSelectedAnnualId(const QString& id)
{
    selection_->setSelectedAnnualId(id);
}

ActorSelection* StateFacade::selectedActor() { return selection_->selectedActor(); }
PropertySelection* StateFacade::selectedProperty() { return selection_->selectedProperty(); }
ContractSelection* StateFacade::selectedContract() { return selection_->selectedContract(); }
StatementSelection* StateFacade::selectedStatement() { return selection_->selectedStatement(); }
TransactionSelection* StateFacade::selectedTransaction() { return selection_->selectedTransaction(); }
AnalysisSelection* StateFacade::selectedAnalysis() { return selection_->selectedAnalysis(); }
AnnualSelection* StateFacade::selectedAnnual() { return selection_->selectedAnnual(); }

QVariantList StateFacade::statementTransactionIds(const QString& statementId) const
{
    QVariantList out;
    if (statementId.isEmpty()) return out;
    for (const auto& transaction : session_->models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) == statementId) out.push_back(QString::fromStdString(transaction->id));
    }
    return out;
}

QVariantList StateFacade::contractRows() const
{
    QVariantList out;
    for (const auto& contract : session_->models().contracts().contracts()) {
        if (!contract) continue;

        QStringList aliases;
        for (const auto& alias : contract->aliases) aliases.push_back(QString::fromStdString(alias));

        QVariantMap row;
        row.insert("id", QString::fromStdString(contract->id));
        row.insert("name", QString::fromStdString(contract->name));
        row.insert("type", QString::fromStdString(contract->type));
        row.insert("display", QString::fromStdString(contract->name));
        row.insert("aliases", aliases);
        QStringList actorIds;
        for (const auto& actorId : contract->actorIds) actorIds.push_back(QString::fromStdString(actorId));
        QStringList propertyIds;
        for (const auto& propertyId : contract->propertyIds) propertyIds.push_back(QString::fromStdString(propertyId));
        row.insert("actorIds", actorIds);
        row.insert("propertyIds", propertyIds);
        out.push_back(row);
    }
    return out;
}

QVariantList StateFacade::actorRows() const
{
    QVariantList out;
    for (const auto& actor : session_->models().actors().actors()) {
        if (!actor) continue;

        QStringList aliases;
        for (const auto& alias : actor->aliases) aliases.push_back(QString::fromStdString(alias));

        QVariantMap row;
        row.insert("id", QString::fromStdString(actor->id));
        row.insert("name", QString::fromStdString(actor->name));
        row.insert("type", QString::fromStdString(actor->type));
        row.insert("display", actor->type.empty()
                                 ? QString::fromStdString(actor->name)
                                 : QString::fromStdString(actor->name) + QStringLiteral(" — ") + QString::fromStdString(actor->type));
        row.insert("aliases", aliases);
        out.push_back(row);
    }
    return out;
}

QVariantList StateFacade::propertyRows() const
{
    QVariantList out;
    for (const auto& property : session_->models().properties().properties()) {
        if (!property) continue;

        QStringList aliases;
        for (const auto& alias : property->aliases) aliases.push_back(QString::fromStdString(alias));

        QVariantMap row;
        row.insert("id", QString::fromStdString(property->id));
        row.insert("name", QString::fromStdString(property->name));
        row.insert("display", property->address.empty()
                                 ? QString::fromStdString(property->name)
                                 : QString::fromStdString(property->name) + QStringLiteral(" — ") + QString::fromStdString(property->address));
        row.insert("aliases", aliases);
        out.push_back(row);
    }
    return out;
}

QVariantList StateFacade::analysisRows() const
{
    QVariantList out;
    for (const auto& analysis : session_->models().analyses().analyses()) {
        if (!analysis) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(analysis->id));
        row.insert("name", QString::fromStdString(analysis->name));
        row.insert("type", QString::fromStdString(analysis->type));
        out.push_back(row);
    }
    return out;
}

QVariantList StateFacade::statementRows() const
{
    QVariantList out;
    for (const auto& statement : session_->models().statements().statements()) {
        if (!statement) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(statement->id));
        row.insert("name", QString::fromStdString(statement->name));
        out.push_back(row);
    }
    return out;
}

QVariantList StateFacade::statementTransactionRows(const QString& statementId) const
{
    QVariantList out;
    if (statementId.isEmpty()) return out;

    for (const auto& transaction : session_->models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) != statementId) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(transaction->id));
        row.insert("name", QString::fromStdString(transaction->name));
        row.insert("bookingDate", QString::fromStdString(transaction->bookingDate));
        out.push_back(row);
    }
    return out;
}

TransactionFilter* StateFacade::statementTransactions(const QString& statementId)
{
    return session_->statementTransactions(statementId, this);
}

TransactionFilter* StateFacade::propertyTransactions(const QString& propertyId)
{
    return session_->propertyTransactions(propertyId, this);
}

QStringList StateFacade::propertyContractTypes(const QString& propertyId) const
{
    return session_->propertyContractTypes(propertyId);
}

QVariantMap StateFacade::propertyTransactionSums(const QString& propertyId, const QString& contractType) const
{
    return session_->propertyTransactionSums(propertyId, contractType);
}

QString StateFacade::propertyName(const QString& id) const
{
    return session_->propertyName(id);
}

void StateFacade::applyDeletionImpact(const DeletionImpact& impact)
{
    session_->applyDeletionImpact(impact);
}

void StateFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    session_->setTransactionPropertyIdsImmediate(txId, propertyIds);
}

}
