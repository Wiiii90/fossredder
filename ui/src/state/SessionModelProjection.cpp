/**
 * @file ui/src/state/SessionModelProjection.cpp
 * @brief Implements helpers that project core app state into UI session models.
 */

#include "ui/state/SessionModelProjection.h"

#include <utility>

#include "ui/state/WorkspaceClone.h"
#include "ui/state/SessionModels.h"

namespace ui {

QHash<QString, QString> buildContractTypeIndex(const ContractList& contracts)
{
    QHash<QString, QString> contractTypes;
    const auto& rows = contracts.contracts();
    contractTypes.reserve(static_cast<int>(rows.size()));
    for (const auto& contract : rows) {
        if (!contract) continue;
        contractTypes.insert(QString::fromStdString(contract->id()), QString::fromStdString(contract->type()));
    }
    return contractTypes;
}

void projectSessionModelsFromState(SessionModels& models, const core::domain::catalog::WorkspaceCatalog& state)
{
    core::domain::catalog::WorkspaceCatalog clone = cloneWorkspaceState(state);

    models.actors().setActors(std::move(clone.actors()));
    models.properties().setProperties(std::move(clone.properties()));
    models.contracts().setContracts(std::move(clone.contracts()));
    models.transactions().setContractTypes(buildContractTypeIndex(models.contracts()), false);
    models.statements().setStatements(std::move(clone.statements()));
    models.transactions().setTransactions(std::move(clone.transactions()));
    models.analyses().setAnalyses(std::move(clone.analyses()));
    models.annuals().setAnnuals(std::move(clone.annuals()));
}

void refreshSessionModelContractTypes(SessionModels& models)
{
    models.transactions().setContractTypes(buildContractTypeIndex(models.contracts()));
}

} // namespace ui
