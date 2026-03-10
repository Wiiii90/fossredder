#include "ui/state/SessionModels.h"

#include <utility>

#include "ui/state/AppStateClone.h"

namespace ui {

namespace {

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

}

SessionModels::SessionModels(QObject* parent)
    : actors_(parent)
    , analyses_(parent)
    , properties_(parent)
    , contracts_(parent)
    , statements_(parent)
    , transactions_(parent)
    , annuals_(parent)
{
}

void SessionModels::loadFromState(const AppState& state)
{
    AppState clone = cloneAppState(state);
    actors_.setActors(std::move(clone.actors));
    properties_.setProperties(std::move(clone.properties));
    contracts_.setContracts(std::move(clone.contracts));
    statements_.setStatements(std::move(clone.statements));
    transactions_.setTransactions(std::move(clone.transactions));
    analyses_.setAnalyses(std::move(clone.analyses));
    annuals_.setAnnuals(std::move(clone.annuals));
    refreshContractTypes();
}

void SessionModels::refreshContractTypes()
{
    transactions_.setContractTypes(buildContractTypeIndex(contracts_));
}

}
