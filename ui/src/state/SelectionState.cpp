#include "ui/state/SelectionState.h"

#include <utility>

namespace ui {

template <typename Selection, typename Collection, typename Mapper>
void refreshSelection(const QString& selectedId,
                      const Collection& items,
                      Selection& selection,
                      Mapper&& mapper)
{
    if (selectedId.isEmpty()) { selection.clear(); return; }
    for (const auto& item : items) {
        if (!item) continue;
        if (QString::fromStdString(item->id) != selectedId) continue;
        mapper(*item, selection);
        return;
    }
    selection.clear();
}

ActorSelection::ActorSelection(QObject* parent)
    : QObject(parent)
{
}

void ActorSelection::clear()
{
    set({}, {}, {}, {}, {});
}

void ActorSelection::set(QString id, QString name, QString type, QString description, QStringList aliases)
{
    id_ = std::move(id);
    name_ = std::move(name);
    type_ = std::move(type);
    description_ = std::move(description);
    aliases_ = std::move(aliases);
    emit changed();
}

PropertySelection::PropertySelection(QObject* parent)
    : QObject(parent)
{
}

void PropertySelection::clear()
{
    set({}, {}, {}, {});
}

void PropertySelection::set(QString id, QString name, QString address, QString description)
{
    id_ = std::move(id);
    name_ = std::move(name);
    address_ = std::move(address);
    description_ = std::move(description);
    emit changed();
}

ContractSelection::ContractSelection(QObject* parent)
    : QObject(parent)
{
}

void ContractSelection::clear()
{
    set({}, {}, {}, {}, {}, {});
}

void ContractSelection::set(QString id,
                            QString name,
                            QString type,
                            QString description,
                            QStringList actorIds,
                            QStringList propertyIds)
{
    id_ = std::move(id);
    name_ = std::move(name);
    type_ = std::move(type);
    description_ = std::move(description);
    actorIds_ = std::move(actorIds);
    propertyIds_ = std::move(propertyIds);
    emit changed();
}

StatementSelection::StatementSelection(QObject* parent)
    : QObject(parent)
{
}

void StatementSelection::clear()
{
    set({}, {});
}

void StatementSelection::set(QString id, QString name)
{
    id_ = std::move(id);
    name_ = std::move(name);
    emit changed();
}

TransactionSelection::TransactionSelection(QObject* parent)
    : QObject(parent)
{
}

void TransactionSelection::clear()
{
    set({}, {}, {}, 0.0, {}, {}, {}, {}, {}, false);
}

void TransactionSelection::set(QString id,
                               QString name,
                               QString bookingDate,
                               double amount,
                               QString description,
                               QString statementId,
                               QString actorId,
                               QString actorProposal,
                               QStringList propertyIds,
                               bool allocatable)
{
    id_ = std::move(id);
    name_ = std::move(name);
    bookingDate_ = std::move(bookingDate);
    amount_ = amount;
    description_ = std::move(description);
    statementId_ = std::move(statementId);
    actorId_ = std::move(actorId);
    actorProposal_ = std::move(actorProposal);
    propertyIds_ = std::move(propertyIds);
    allocatable_ = allocatable;
    emit changed();
}

AnalysisSelection::AnalysisSelection(QObject* parent)
    : QObject(parent)
{
}

void AnalysisSelection::clear()
{
    set({}, {}, {});
}

void AnalysisSelection::set(QString id, QString name, QString type)
{
    id_ = std::move(id);
    name_ = std::move(name);
    type_ = std::move(type);
    emit changed();
}

AnnualSelection::AnnualSelection(QObject* parent)
    : QObject(parent)
{
}

void AnnualSelection::clear()
{
    set({}, {});
}

void AnnualSelection::set(QString id, QString name)
{
    id_ = std::move(id);
    name_ = std::move(name);
    emit changed();
}

SelectionState::SelectionState(ActorList& actors,
                               PropertyList& properties,
                               ContractList& contracts,
                               StatementList& statements,
                               TransactionList& transactions,
                               AnalysisList& analyses,
                               AnnualList& annuals,
                               QObject* parent)
    : actors_(actors)
    , properties_(properties)
    , contracts_(contracts)
    , statements_(statements)
    , transactions_(transactions)
    , analyses_(analyses)
    , annuals_(annuals)
    , selectedActor_(parent)
    , selectedProperty_(parent)
    , selectedContract_(parent)
    , selectedStatement_(parent)
    , selectedTransaction_(parent)
    , selectedAnalysis_(parent)
    , selectedAnnual_(parent)
{
}

const QString& SelectionState::selectedActorId() const { return selectedActorId_; }
const QString& SelectionState::selectedPropertyId() const { return selectedPropertyId_; }
const QString& SelectionState::selectedContractId() const { return selectedContractId_; }
const QString& SelectionState::selectedStatementId() const { return selectedStatementId_; }
const QString& SelectionState::selectedTransactionId() const { return selectedTransactionId_; }
const QString& SelectionState::selectedAnalysisId() const { return selectedAnalysisId_; }
const QString& SelectionState::selectedAnnualId() const { return selectedAnnualId_; }

bool SelectionState::setSelectedActorId(const QString& id) { if (selectedActorId_ == id) return false; selectedActorId_ = id; refreshSelectedActor(); return true; }
bool SelectionState::setSelectedPropertyId(const QString& id) { if (selectedPropertyId_ == id) return false; selectedPropertyId_ = id; refreshSelectedProperty(); return true; }
bool SelectionState::setSelectedContractId(const QString& id) { if (selectedContractId_ == id) return false; selectedContractId_ = id; refreshSelectedContract(); return true; }
bool SelectionState::setSelectedStatementId(const QString& id) { if (selectedStatementId_ == id) return false; selectedStatementId_ = id; refreshSelectedStatement(); return true; }
bool SelectionState::setSelectedTransactionId(const QString& id) { if (selectedTransactionId_ == id) return false; selectedTransactionId_ = id; refreshSelectedTransaction(); return true; }
bool SelectionState::setSelectedAnalysisId(const QString& id) { if (selectedAnalysisId_ == id) return false; selectedAnalysisId_ = id; refreshSelectedAnalysis(); return true; }
bool SelectionState::setSelectedAnnualId(const QString& id) { if (selectedAnnualId_ == id) return false; selectedAnnualId_ = id; refreshSelectedAnnual(); return true; }

ActorSelection* SelectionState::selectedActor() { return &selectedActor_; }
PropertySelection* SelectionState::selectedProperty() { return &selectedProperty_; }
ContractSelection* SelectionState::selectedContract() { return &selectedContract_; }
StatementSelection* SelectionState::selectedStatement() { return &selectedStatement_; }
TransactionSelection* SelectionState::selectedTransaction() { return &selectedTransaction_; }
AnalysisSelection* SelectionState::selectedAnalysis() { return &selectedAnalysis_; }
AnnualSelection* SelectionState::selectedAnnual() { return &selectedAnnual_; }

void SelectionState::refreshAll()
{
    refreshSelectedActor();
    refreshSelectedProperty();
    refreshSelectedContract();
    refreshSelectedStatement();
    refreshSelectedTransaction();
    refreshSelectedAnalysis();
    refreshSelectedAnnual();
}

void SelectionState::refreshSelectedActor()
{
    refreshSelection(selectedActorId_, actors_.actors(), selectedActor_, [](const Actor& actor, ActorSelection& selection) {
        QStringList aliases;
        for (const auto& alias : actor.aliases) aliases.push_back(QString::fromStdString(alias));
        selection.set(QString::fromStdString(actor.id),
                      QString::fromStdString(actor.name),
                      QString::fromStdString(actor.type),
                      QString::fromStdString(actor.description),
                      aliases);
    });
}

void SelectionState::refreshSelectedProperty()
{
    refreshSelection(selectedPropertyId_, properties_.properties(), selectedProperty_, [](const Property& property, PropertySelection& selection) {
        selection.set(QString::fromStdString(property.id),
                      QString::fromStdString(property.name),
                      QString::fromStdString(property.address),
                      QString::fromStdString(property.description));
    });
}

void SelectionState::refreshSelectedContract()
{
    refreshSelection(selectedContractId_, contracts_.contracts(), selectedContract_, [](const Contract& contract, ContractSelection& selection) {
        QStringList actorIds;
        for (const auto& actorId : contract.actorIds) actorIds.push_back(QString::fromStdString(actorId));
        QStringList propertyIds;
        for (const auto& propertyId : contract.propertyIds) propertyIds.push_back(QString::fromStdString(propertyId));
        selection.set(QString::fromStdString(contract.id),
                      QString::fromStdString(contract.name),
                      QString::fromStdString(contract.type),
                      QString::fromStdString(contract.description),
                      actorIds,
                      propertyIds);
    });
}

void SelectionState::refreshSelectedStatement()
{
    refreshSelection(selectedStatementId_, statements_.statements(), selectedStatement_, [](const Statement& statement, StatementSelection& selection) {
        selection.set(QString::fromStdString(statement.id), QString::fromStdString(statement.name));
    });
}

void SelectionState::refreshSelectedTransaction()
{
    refreshSelection(selectedTransactionId_, transactions_.transactions(), selectedTransaction_, [](const Transaction& transaction, TransactionSelection& selection) {
        QStringList propertyIds;
        for (const auto& propertyId : transaction.propertyIds) propertyIds.push_back(QString::fromStdString(propertyId));
        selection.set(QString::fromStdString(transaction.id),
                      QString::fromStdString(transaction.name),
                      QString::fromStdString(transaction.bookingDate),
                      transaction.amount,
                      QString::fromStdString(transaction.description),
                      QString::fromStdString(transaction.statementId),
                      QString::fromStdString(transaction.actorId),
                      QString::fromStdString(transaction.actorProposal),
                      propertyIds,
                      transaction.allocatable);
    });
}

void SelectionState::refreshSelectedAnalysis()
{
    refreshSelection(selectedAnalysisId_, analyses_.analyses(), selectedAnalysis_, [](const Analysis& analysis, AnalysisSelection& selection) {
        selection.set(QString::fromStdString(analysis.id),
                      QString::fromStdString(analysis.name),
                      QString::fromStdString(analysis.type));
    });
}

void SelectionState::refreshSelectedAnnual()
{
    refreshSelection(selectedAnnualId_, annuals_.annuals(), selectedAnnual_, [](const Annual& annual, AnnualSelection& selection) {
        selection.set(QString::fromStdString(annual.id), QString::number(annual.year));
    });
}

bool SelectionState::clearActorIfSelected(const QString& id)
{
    if (selectedActorId_ != id) return false;
    selectedActorId_.clear();
    selectedActor_.clear();
    return true;
}

bool SelectionState::clearPropertyIfSelected(const QString& id)
{
    if (selectedPropertyId_ != id) return false;
    selectedPropertyId_.clear();
    selectedProperty_.clear();
    return true;
}

bool SelectionState::clearContractIfSelected(const QString& id)
{
    if (selectedContractId_ != id) return false;
    selectedContractId_.clear();
    selectedContract_.clear();
    return true;
}

bool SelectionState::clearStatementIfSelected(const QString& id)
{
    if (selectedStatementId_ != id) return false;
    selectedStatementId_.clear();
    selectedStatement_.clear();
    return true;
}

bool SelectionState::clearTransactionIfSelected(const QString& id)
{
    if (selectedTransactionId_ != id) return false;
    selectedTransactionId_.clear();
    selectedTransaction_.clear();
    return true;
}

}
