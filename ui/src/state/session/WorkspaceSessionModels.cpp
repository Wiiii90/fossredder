/**
 * @file ui/src/state/SessionModels.cpp
 * @brief Implementation of the UI SessionModels component.
 */

#include "ui/state/session/WorkspaceSessionModels.h"

#include <QHash>
#include <QString>
#include <utility>

namespace ui {

SessionModels::SessionModels(QObject* objectParent)
    : actors_(std::make_unique<ActorList>(objectParent))
    , analyses_(std::make_unique<AnalysisList>(objectParent))
    , properties_(std::make_unique<PropertyList>(objectParent))
    , contracts_(std::make_unique<ContractList>(objectParent))
    , statements_(std::make_unique<StatementList>(objectParent))
    , transactions_(std::make_unique<TransactionList>(objectParent))
    , annuals_(std::make_unique<AnnualList>(objectParent))
{
}

void SessionModels::loadFromState(const core::domain::catalog::WorkspaceCatalog& state)
{
    const auto& stateContracts = state.contracts();
    QHash<QString, QString> contractTypes;
    contractTypes.reserve(static_cast<int>(stateContracts.size()));
    for (const auto& contract : stateContracts) {
        if (!contract) continue;
        contractTypes.insert(QString::fromStdString(contract->id()), QString::fromStdString(contract->type()));
    }

    actors().setActors(state.actors());
    properties().setProperties(state.properties());
    contracts().setContracts(state.contracts());
    transactions().setContractTypes(std::move(contractTypes), false);
    statements().setStatements(state.statements());
    transactions().setTransactions(state.transactions());
    analyses().setAnalyses(state.analyses());
    annuals().setAnnuals(state.annuals());
}

void SessionModels::refreshContractTypes()
{
    QHash<QString, QString> contractTypes;
    const auto& rows = contracts().contracts();
    contractTypes.reserve(static_cast<int>(rows.size()));
    for (const auto& contract : rows) {
        if (!contract) continue;
        contractTypes.insert(QString::fromStdString(contract->id()), QString::fromStdString(contract->type()));
    }
    transactions().setContractTypes(std::move(contractTypes));
}

}
