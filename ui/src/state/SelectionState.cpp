#include "ui/state/SelectionState.h"

namespace ui {

EntitySelection::EntitySelection(QObject* parent)
    : QObject(parent)
{
}

void EntitySelection::clear()
{
    id_.clear();
    name_.clear();
    type_.clear();
    aliases_.clear();
    address_.clear();
    description_.clear();
    actorIds_.clear();
    propertyIds_.clear();
    bookingDate_.clear();
    amount_ = 0.0;
    allocatable_ = false;
    statementId_.clear();
    actorId_.clear();
    actorProposal_.clear();
    emit changed();
}

void EntitySelection::setActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    id_ = id;
    name_ = name;
    type_ = type;
    aliases_ = aliases;
    address_.clear();
    description_ = description;
    actorIds_.clear();
    propertyIds_.clear();
    bookingDate_.clear();
    amount_ = 0.0;
    statementId_.clear();
    actorId_.clear();
    actorProposal_.clear();
    emit changed();
}

void EntitySelection::setProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    id_ = id;
    name_ = name;
    type_.clear();
    aliases_.clear();
    address_ = address;
    description_ = description;
    actorIds_.clear();
    propertyIds_.clear();
    bookingDate_.clear();
    amount_ = 0.0;
    statementId_.clear();
    actorId_.clear();
    actorProposal_.clear();
    emit changed();
}

void EntitySelection::setStatement(const QString& id, const QString& name)
{
    id_ = id;
    name_ = name;
    type_.clear();
    aliases_.clear();
    address_.clear();
    description_.clear();
    actorIds_.clear();
    propertyIds_.clear();
    bookingDate_.clear();
    amount_ = 0.0;
    statementId_.clear();
    actorId_.clear();
    actorProposal_.clear();
    emit changed();
}

void EntitySelection::setTransaction(const QString& id,
                                     const QString& name,
                                     const QString& bookingDate,
                                     double amount,
                                     const QString& description,
                                     const QString& statementId,
                                     const QString& actorId,
                                     const QString& actorProposal,
                                     const QStringList& propertyIds,
                                     bool allocatable,
                                     const QString& transactionType)
{
    id_ = id;
    name_ = name;
    type_.clear();
    aliases_.clear();
    address_.clear();
    description_ = description;
    actorIds_.clear();
    propertyIds_ = propertyIds;
    bookingDate_ = bookingDate;
    amount_ = amount;
    statementId_ = statementId;
    actorId_ = actorId;
    actorProposal_ = actorProposal;
    allocatable_ = allocatable;
    type_ = transactionType;
    emit changed();
}

void EntitySelection::setContract(const QString& id, const QString& name, const QString& type, const QString& description)
{
    setContract(id, name, type, description, {}, {});
}

void EntitySelection::setContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                  const QStringList& actorIds, const QStringList& propertyIds)
{
    id_ = id;
    name_ = name;
    type_ = type;
    aliases_.clear();
    address_.clear();
    description_ = description;
    actorIds_ = actorIds;
    propertyIds_ = propertyIds;
    bookingDate_.clear();
    amount_ = 0.0;
    statementId_.clear();
    actorId_.clear();
    actorProposal_.clear();
    allocatable_ = false;
    emit changed();
}

void EntitySelection::setAnalysis(const QString& id, const QString& name, const QString& type, const QString& description)
{
    id_ = id;
    name_ = name;
    type_ = type;
    aliases_.clear();
    address_.clear();
    description_ = description;
    actorIds_.clear();
    propertyIds_.clear();
    bookingDate_.clear();
    amount_ = 0.0;
    statementId_.clear();
    actorId_.clear();
    actorProposal_.clear();
    allocatable_ = false;
    emit changed();
}

void EntitySelection::setAnnual(const QString& id, int year)
{
    id_ = id;
    name_ = QString::number(year);
    type_.clear();
    aliases_.clear();
    address_.clear();
    description_.clear();
    actorIds_.clear();
    propertyIds_.clear();
    bookingDate_.clear();
    amount_ = 0.0;
    statementId_.clear();
    actorId_.clear();
    actorProposal_.clear();
    allocatable_ = false;
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

EntitySelection* SelectionState::selectedActor() { return &selectedActor_; }
EntitySelection* SelectionState::selectedProperty() { return &selectedProperty_; }
EntitySelection* SelectionState::selectedContract() { return &selectedContract_; }
EntitySelection* SelectionState::selectedStatement() { return &selectedStatement_; }
EntitySelection* SelectionState::selectedTransaction() { return &selectedTransaction_; }
EntitySelection* SelectionState::selectedAnalysis() { return &selectedAnalysis_; }
EntitySelection* SelectionState::selectedAnnual() { return &selectedAnnual_; }

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
    if (selectedActorId_.isEmpty()) { selectedActor_.clear(); return; }
    for (const auto& a : actors_.actors()) {
        if (!a) continue;
        if (QString::fromStdString(a->id) != selectedActorId_) continue;
        QStringList aliases;
        for (const auto& al : a->aliases) aliases.push_back(QString::fromStdString(al));
        selectedActor_.setActor(QString::fromStdString(a->id),
                                QString::fromStdString(a->name),
                                QString::fromStdString(a->type),
                                QString::fromStdString(a->description),
                                aliases);
        return;
    }
    selectedActor_.clear();
}

void SelectionState::refreshSelectedProperty()
{
    if (selectedPropertyId_.isEmpty()) { selectedProperty_.clear(); return; }
    for (const auto& p : properties_.properties()) {
        if (!p) continue;
        if (QString::fromStdString(p->id) != selectedPropertyId_) continue;
        selectedProperty_.setProperty(QString::fromStdString(p->id),
                                      QString::fromStdString(p->name),
                                      QString::fromStdString(p->address),
                                      QString::fromStdString(p->description));
        return;
    }
    selectedProperty_.clear();
}

void SelectionState::refreshSelectedContract()
{
    if (selectedContractId_.isEmpty()) { selectedContract_.clear(); return; }
    for (const auto& c : contracts_.contracts()) {
        if (!c) continue;
        if (QString::fromStdString(c->id) != selectedContractId_) continue;
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
    selectedContract_.clear();
}

void SelectionState::refreshSelectedStatement()
{
    if (selectedStatementId_.isEmpty()) { selectedStatement_.clear(); return; }
    for (const auto& s : statements_.statements()) {
        if (!s) continue;
        if (QString::fromStdString(s->id) != selectedStatementId_) continue;
        selectedStatement_.setStatement(QString::fromStdString(s->id), QString::fromStdString(s->name));
        return;
    }
    selectedStatement_.clear();
}

void SelectionState::refreshSelectedTransaction()
{
    if (selectedTransactionId_.isEmpty()) { selectedTransaction_.clear(); return; }
    for (const auto& t : transactions_.transactions()) {
        if (!t) continue;
        if (QString::fromStdString(t->id) != selectedTransactionId_) continue;
        QStringList propertyIds;
        for (const auto& pid : t->propertyIds) propertyIds.push_back(QString::fromStdString(pid));
        selectedTransaction_.setTransaction(QString::fromStdString(t->id),
                                            QString::fromStdString(t->name),
                                            QString::fromStdString(t->bookingDate),
                                            t->amount,
                                            QString::fromStdString(t->description),
                                            QString::fromStdString(t->statementId),
                                            QString::fromStdString(t->actorId),
                                            QString::fromStdString(t->actorProposal),
                                            propertyIds,
                                            t->allocatable,
                                            QString());
        return;
    }
    selectedTransaction_.clear();
}

void SelectionState::refreshSelectedAnalysis()
{
    if (selectedAnalysisId_.isEmpty()) { selectedAnalysis_.clear(); return; }
    for (const auto& a : analyses_.analyses()) {
        if (!a) continue;
        if (QString::fromStdString(a->id) != selectedAnalysisId_) continue;
        selectedAnalysis_.setAnalysis(QString::fromStdString(a->id),
                                      QString::fromStdString(a->name),
                                      QString::fromStdString(a->type));
        return;
    }
    selectedAnalysis_.clear();
}

void SelectionState::refreshSelectedAnnual()
{
    if (selectedAnnualId_.isEmpty()) { selectedAnnual_.clear(); return; }
    for (const auto& an : annuals_.annuals()) {
        if (!an) continue;
        if (QString::fromStdString(an->id) != selectedAnnualId_) continue;
        selectedAnnual_.setAnnual(QString::fromStdString(an->id), an->year);
        return;
    }
    selectedAnnual_.clear();
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
