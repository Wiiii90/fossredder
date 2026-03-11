#include "core/application/CatalogService.h"

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/utils/StableId.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace {

template <typename Collection, typename Configure>
std::string appendEntity(Collection& collection, Configure&& configure)
{
    using Entity = typename Collection::value_type::element_type;
    auto entity = std::make_shared<Entity>();
    entity->id = core::utils::makeStableId();
    configure(*entity);
    const auto id = entity->id;
    collection.push_back(std::move(entity));
    return id;
}

template <typename Collection, typename Update>
bool updateEntity(Collection& collection, const std::string& id, Update&& update)
{
    if (id.empty()) return false;
    for (auto& entity : collection) {
        if (!entity || entity->id != id) continue;
        update(*entity);
        return true;
    }
    return false;
}

template <typename Collection>
bool eraseEntity(Collection& collection, const std::string& id)
{
    if (id.empty()) return false;
    const auto originalSize = collection.size();
    collection.erase(std::remove_if(collection.begin(), collection.end(), [&](const auto& entity) {
        return entity && entity->id == id;
    }), collection.end());
    return collection.size() != originalSize;
}

void resetTransientTransactionFields(Transaction& tx)
{
    tx.valuta.clear();
}

void applyActorDraft(Actor& actor,
                     const std::string& name,
                     const std::string& type,
                     const std::string& description)
{
    actor.name = name;
    actor.type = type;
    actor.description = description;
}

void applyPropertyDraft(Property& property,
                        const std::string& name,
                        const std::string& address,
                        const std::string& description)
{
    property.name = name;
    property.address = address;
    property.description = description;
}

void applyContractDraft(Contract& contract,
                        const std::string& name,
                        const std::string& type,
                        const std::string& description,
                        const std::vector<std::string>& actorIds,
                        const std::vector<std::string>& propertyIds)
{
    contract.name = name;
    contract.type = type;
    contract.description = description;
    contract.actorIds = actorIds;
    contract.propertyIds = propertyIds;
}

void applyStatementName(Statement& statement, const std::string& name)
{
    statement.name = name;
}

void applyAnalysisDraft(Analysis& analysis,
                        const std::string& name,
                        const std::string& type,
                        const std::string& configJson,
                        const std::string& filterSpec)
{
    analysis.name = name;
    analysis.type = type;
    analysis.configJson = configJson;
    analysis.filterSpec = filterSpec;
}

void applyAnnualDraft(Annual& annual, int year)
{
    annual.year = year;
}

void applyTransactionDraft(Transaction& tx,
                          const std::string& name,
                          const std::string& bookingDate,
                          double amount,
                          const std::string& description,
                          const std::string& statementId,
                          Transaction::Status status,
                          const std::string& actorId,
                          bool allocatable,
                          const std::vector<std::string>& propertyIds)
{
    tx.name = name;
    tx.bookingDate = bookingDate;
    tx.amount = amount;
    tx.description = description;
    tx.statementId = statementId;
    tx.status = status;
    tx.actorId = actorId;
    tx.allocatable = allocatable;
    tx.propertyIds = propertyIds;
}

bool isBlank(const std::string& value)
{
    return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; });
}

}

namespace core::application {

std::string CatalogService::addActor(AppState& state, const std::string& name, const std::string& type, const std::string& description) const
{
    return appendEntity(state.actors, [&](Actor& actor) { applyActorDraft(actor, name, type, description); });
}

bool CatalogService::updateActor(AppState& state, const std::string& id, const std::string& name, const std::string& type, const std::string& description) const
{
    return updateEntity(state.actors, id, [&](Actor& actor) { applyActorDraft(actor, name, type, description); });
}

bool CatalogService::deleteActor(AppState& state, const std::string& id) const
{
    return eraseEntity(state.actors, id);
}

std::string CatalogService::addProperty(AppState& state, const std::string& name, const std::string& address, const std::string& description) const
{
    return appendEntity(state.properties, [&](Property& property) { applyPropertyDraft(property, name, address, description); });
}

bool CatalogService::updateProperty(AppState& state, const std::string& id, const std::string& name, const std::string& address, const std::string& description) const
{
    return updateEntity(state.properties, id, [&](Property& property) { applyPropertyDraft(property, name, address, description); });
}

bool CatalogService::deleteProperty(AppState& state, const std::string& id) const
{
    return eraseEntity(state.properties, id);
}

std::string CatalogService::addContract(AppState& state, const std::string& name, const std::string& type, const std::string& description,
                                        const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds) const
{
    return appendEntity(state.contracts, [&](Contract& contract) { applyContractDraft(contract, name, type, description, actorIds, propertyIds); });
}

bool CatalogService::updateContract(AppState& state, const std::string& id, const std::string& name, const std::string& type, const std::string& description,
                                    const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds) const
{
    return updateEntity(state.contracts, id, [&](Contract& contract) { applyContractDraft(contract, name, type, description, actorIds, propertyIds); });
}

bool CatalogService::deleteContract(AppState& state, const std::string& id) const
{
    return eraseEntity(state.contracts, id);
}

std::string CatalogService::addStatement(AppState& state, const std::string& name) const
{
    return appendEntity(state.statements, [&](Statement& statement) { applyStatementName(statement, name); });
}

bool CatalogService::updateStatement(AppState& state, const std::string& id, const std::string& name) const
{
    return updateEntity(state.statements, id, [&](Statement& statement) { applyStatementName(statement, name); });
}

bool CatalogService::deleteStatement(AppState& state, const std::string& id) const
{
    if (id.empty()) return false;

    const auto originalTransactionCount = state.transactions.size();
    state.transactions.erase(std::remove_if(state.transactions.begin(), state.transactions.end(), [&](const auto& tx) {
        return tx && tx->statementId == id;
    }), state.transactions.end());

    const bool removedStatement = eraseEntity(state.statements, id);
    return removedStatement || state.transactions.size() != originalTransactionCount;
}

std::string CatalogService::addTransaction(AppState& state, const std::string& name, const std::string& bookingDate, double amount,
                                           const std::string& description, const std::string& statementId, Transaction::Status status,
                                           const std::string& actorId, bool allocatable, const std::vector<std::string>& propertyIds) const
{
    if (isBlank(statementId)) return {};

    return appendEntity(state.transactions, [&](Transaction& tx) {
        applyTransactionDraft(tx, name, bookingDate, amount, description, statementId, status, actorId, allocatable, propertyIds);
        resetTransientTransactionFields(tx);
    });
}

bool CatalogService::updateTransaction(AppState& state, const std::string& id, const std::string& name, const std::string& bookingDate, double amount,
                                       const std::string& description, const std::string& statementId, Transaction::Status status,
                                       const std::string& actorId, bool allocatable, const std::vector<std::string>& propertyIds) const
{
    return updateEntity(state.transactions, id, [&](Transaction& tx) {
        applyTransactionDraft(tx, name, bookingDate, amount, description, statementId, status, actorId, allocatable, propertyIds);
    });
}

bool CatalogService::deleteTransaction(AppState& state, const std::string& id) const
{
    return eraseEntity(state.transactions, id);
}

std::string CatalogService::addAnalysis(AppState& state, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec) const
{
    return appendEntity(state.analyses, [&](Analysis& analysis) { applyAnalysisDraft(analysis, name, type, configJson, filterSpec); });
}

bool CatalogService::updateAnalysis(AppState& state, const std::string& id, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec) const
{
    return updateEntity(state.analyses, id, [&](Analysis& analysis) { applyAnalysisDraft(analysis, name, type, configJson, filterSpec); });
}

bool CatalogService::deleteAnalysis(AppState& state, const std::string& id) const
{
    return eraseEntity(state.analyses, id);
}

std::string CatalogService::addAnnual(AppState& state, int year) const
{
    return appendEntity(state.annuals, [&](Annual& annual) { applyAnnualDraft(annual, year); });
}

bool CatalogService::updateAnnual(AppState& state, const std::string& id, int year) const
{
    return updateEntity(state.annuals, id, [&](Annual& annual) { applyAnnualDraft(annual, year); });
}

bool CatalogService::deleteAnnual(AppState& state, const std::string& id) const
{
    return eraseEntity(state.annuals, id);
}

std::vector<std::string> CatalogService::contractTypes(const AppState& state) const
{
    std::unordered_set<std::string> seen;
    std::vector<std::string> values;
    values.reserve(state.contracts.size());

    for (const auto& contract : state.contracts) {
        if (!contract || contract->type.empty()) continue;
        if (!seen.insert(contract->type).second) continue;
        values.push_back(contract->type);
    }

    std::sort(values.begin(), values.end());
    return values;
}

}
