#include "core/application/StateProjector.h"

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

#include <algorithm>

namespace {

template <typename TargetCollection, typename SourceCollection, typename Clone>
void cloneCollection(TargetCollection& target, const SourceCollection& source, Clone&& clone)
{
    target.reserve(source.size());
    for (const auto& item : source) {
        target.push_back(clone(item));
    }
}

void dedupStrings(std::vector<std::string>& values)
{
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

void syncContractRelations(Contract& contract)
{
    dedupStrings(contract.actorIds);
    dedupStrings(contract.propertyIds);
}

std::shared_ptr<Actor> cloneActor(const std::shared_ptr<Actor>& actor)
{
    return actor ? std::make_shared<Actor>(*actor) : nullptr;
}

std::shared_ptr<Property> cloneProperty(const std::shared_ptr<Property>& property)
{
    return property ? std::make_shared<Property>(*property) : nullptr;
}

std::shared_ptr<Contract> cloneContract(const std::shared_ptr<Contract>& contract)
{
    return contract ? std::make_shared<Contract>(*contract) : nullptr;
}

std::shared_ptr<Statement> cloneStatement(const std::shared_ptr<Statement>& statement)
{
    return statement ? std::make_shared<Statement>(*statement) : nullptr;
}

std::shared_ptr<Transaction> cloneTransaction(const std::shared_ptr<Transaction>& transaction)
{
    return transaction ? std::make_shared<Transaction>(*transaction) : nullptr;
}

std::shared_ptr<Analysis> cloneAnalysis(const std::shared_ptr<Analysis>& analysis)
{
    return analysis ? std::make_shared<Analysis>(*analysis) : nullptr;
}

std::shared_ptr<Annual> cloneAnnual(const std::shared_ptr<Annual>& annual)
{
    return annual ? std::make_shared<Annual>(*annual) : nullptr;
}

}

namespace core::application {

AppState StateProjector::prepareForSave(const AppState& state)
{
    AppState projected;

    cloneCollection(projected.actors, state.actors, cloneActor);
    cloneCollection(projected.properties, state.properties, cloneProperty);
    cloneCollection(projected.contracts, state.contracts, cloneContract);
    cloneCollection(projected.statements, state.statements, cloneStatement);
    cloneCollection(projected.transactions, state.transactions, cloneTransaction);
    cloneCollection(projected.analyses, state.analyses, cloneAnalysis);
    cloneCollection(projected.annuals, state.annuals, cloneAnnual);

    for (const auto& contract : projected.contracts) {
        if (!contract) continue;
        syncContractRelations(*contract);
    }

    return projected;
}

}
