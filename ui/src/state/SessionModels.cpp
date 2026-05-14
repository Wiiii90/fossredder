/**
 * @file ui/src/state/SessionModels.cpp
 * @brief Implementation of the UI SessionModels component.
 */

#include "ui/state/SessionModels.h"

#include "ui/state/SessionModelProjection.h"

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
    projectSessionModelsFromState(*this, state);
}

void SessionModels::refreshContractTypes()
{
    refreshSessionModelContractTypes(*this);
}

}
