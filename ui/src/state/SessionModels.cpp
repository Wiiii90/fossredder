#include "ui/state/SessionModels.h"

#include <utility>

#include "ui/state/AppStateClone.h"

namespace ui {

QHash<QString, QString> buildContractTypeIndex(const ContractList& contracts)
{
    QHash<QString, QString> contractTypes;
    const auto& rows = contracts.contracts();
    contractTypes.reserve(static_cast<int>(rows.size()));
    for (const auto& contract : rows) {
        if (!contract) continue;
        contractTypes.insert(QString::fromStdString(contract->id), QString::fromStdString(contract->type));
    }
    return contractTypes;
}

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

void SessionModels::loadFromState(const AppState& state)
{
    AppState clone = cloneAppState(state);

    actors().setActors(std::move(clone.actors));
    properties().setProperties(std::move(clone.properties));
    contracts().setContracts(std::move(clone.contracts));
    transactions().setContractTypes(buildContractTypeIndex(contracts()), false);
    statements().setStatements(std::move(clone.statements));
    transactions().setTransactions(std::move(clone.transactions));
    analyses().setAnalyses(std::move(clone.analyses));
    annuals().setAnnuals(std::move(clone.annuals));
}

void SessionModels::refreshContractTypes()
{
    transactions().setContractTypes(buildContractTypeIndex(contracts()));
}

}
