/**
 * @file core/src/application/analysis/AnalysisService.cpp
 * @brief Implements application-level analysis execution services.
 */

#include "core/application/analysis/AnalysisService.h"

#include "core/constants/analysis.h"
#include "core/application/analysis/AnalysisResult.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Transaction.h"
#include "internal/AnalysisFilter.h"
#include "adjustment/AdjustmentCalculation.h"
#include "presentation/PlotAnalysis.h"
#include "presentation/TableAnalysis.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <utility>

#include <nlohmann/json.hpp>

namespace core::application::analysis {

namespace {

std::vector<AnalysisTransaction> projectAnalysisTransactions(const core::domain::catalog::WorkspaceCatalog& state,
                                                             const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions)
{
    std::vector<AnalysisTransaction> out;
    out.reserve(transactions.size());

    std::unordered_map<std::string, std::shared_ptr<core::domain::Contract>> contractById;
    contractById.reserve(state.contracts().size());
    for (const auto& contract : state.contracts()) {
        if (!contract) {
            continue;
        }
        contractById.emplace(contract->id(), contract);
    }

    std::unordered_map<std::string, std::string> propertyNameById;
    propertyNameById.reserve(state.properties().size());
    for (const auto& property : state.properties()) {
        if (!property) {
            continue;
        }
        propertyNameById.emplace(property->id(), property->name());
    }

    for (const auto& transaction : transactions) {
        if (!transaction) {
            continue;
        }

        AnalysisTransaction projected;
        projected.id = transaction->id();
        projected.name = transaction->name();
        projected.bookingDate = transaction->bookingDate();
        projected.amount = transaction->amount();
        projected.contractId = transaction->contractId();
        projected.propertyIds = transaction->propertyIds();
        if (!transaction->contractId().empty()) {
            const auto it = contractById.find(transaction->contractId());
            if (it != contractById.end() && it->second) {
                projected.contractType = it->second->type();
                for (const auto& propertyId : it->second->propertyIds()) {
                    if (std::find(projected.propertyIds.begin(), projected.propertyIds.end(), propertyId) == projected.propertyIds.end()) {
                        projected.propertyIds.push_back(propertyId);
                    }
                }
            }
        }
        projected.propertyNames.reserve(projected.propertyIds.size());
        for (const auto& propertyId : projected.propertyIds) {
            const auto nameIt = propertyNameById.find(propertyId);
            projected.propertyNames.push_back(nameIt != propertyNameById.end() && !nameIt->second.empty()
                                                  ? nameIt->second
                                                  : propertyId);
        }
        out.push_back(std::move(projected));
    }

    return out;
}

std::shared_ptr<core::domain::catalog::WorkspaceCatalog> catalogWithAdjustments(
    const core::domain::catalog::WorkspaceCatalog& source,
    const std::unordered_map<std::string, double>& adjustments)
{
    if (adjustments.empty()) {
        return {};
    }

    auto catalog = std::make_shared<core::domain::catalog::WorkspaceCatalog>();
    catalog->setContracts(source.contracts());
    catalog->setProperties(source.properties());
    catalog->setAnalyses(source.analyses());
    catalog->setStatements(source.statements());

    core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
    transactions.reserve(source.transactions().size());
    for (const auto& transaction : source.transactions()) {
        if (!transaction) {
            continue;
        }
        auto clone = std::make_shared<core::domain::Transaction>(
            transaction->name(),
            transaction->bookingDate(),
            transaction->valuta(),
            transaction->amount(),
            transaction->isAllocatable());
        clone->setId(transaction->id());
        clone->setStatementId(transaction->statementId());
        clone->setContractId(transaction->contractId());
        clone->setActorId(transaction->actorId());
        clone->setPropertyIds(transaction->propertyIds());
        clone->setStatus(transaction->status());
        clone->setCreatedAt(transaction->createdAt());
        clone->setUpdatedAt(transaction->updatedAt());
        const auto adjustmentIt = adjustments.find(transaction->id());
        if (adjustmentIt != adjustments.end()) {
            clone->setAmount(adjustmentIt->second);
        }
        transactions.push_back(std::move(clone));
    }
    catalog->setTransactions(std::move(transactions));
    return catalog;
}

std::string jsonString(const nlohmann::json& object, const char* key)
{
    const auto it = object.find(key);
    if (it == object.end() || !it->is_string()) {
        return {};
    }
    return it->get<std::string>();
}

double jsonDouble(const nlohmann::json& object, const char* key)
{
    const auto it = object.find(key);
    if (it == object.end() || !it->is_number()) {
        return 0.0;
    }
    return it->get<double>();
}

bool jsonBool(const nlohmann::json& object, const char* key)
{
    const auto it = object.find(key);
    if (it == object.end() || !it->is_boolean()) {
        return false;
    }
    return it->get<bool>();
}

std::vector<std::string> jsonStringList(const nlohmann::json& object, const char* key)
{
    std::vector<std::string> out;
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) {
        return out;
    }
    for (const auto& item : *it) {
        if (!item.is_string()) {
            continue;
        }
        const auto value = item.get<std::string>();
        if (!value.empty() && std::find(out.begin(), out.end(), value) == out.end()) {
            out.push_back(value);
        }
    }
    return out;
}

std::shared_ptr<core::domain::catalog::WorkspaceCatalog> catalogFromAnalysisSnapshot(const core::domain::Analysis& analysis)
{
    if (!analysis.hasSnapshotTransactions()) {
        return {};
    }

    nlohmann::json rows;
    try {
        rows = nlohmann::json::parse(analysis.snapshotTransactionsJson());
    } catch (...) {
        return {};
    }
    if (rows.is_object()) {
        const auto transactions = rows.find("transactions");
        rows = transactions != rows.end() ? *transactions : nlohmann::json{};
    }
    if (!rows.is_array()) {
        return {};
    }

    auto catalog = std::make_shared<core::domain::catalog::WorkspaceCatalog>();
    core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
    core::domain::catalog::WorkspaceCatalog::ContractList contracts;
    core::domain::catalog::WorkspaceCatalog::PropertyList properties;
    std::unordered_set<std::string> seenContracts;
    std::unordered_set<std::string> seenProperties;

    for (std::size_t index = 0; index < rows.size(); ++index) {
        const auto& row = rows[index];
        if (!row.is_object()) {
            continue;
        }

        const std::string explicitId = jsonString(row, "id");
        const std::string transactionId = jsonString(row, "transactionId");
        const std::string id = !explicitId.empty()
            ? explicitId
            : (!transactionId.empty()
                   ? transactionId
                   : "snapshot-transaction-" + std::to_string(index));
        const std::string name = jsonString(row, "name").empty()
            ? jsonString(row, "transactionName")
            : jsonString(row, "name");
        const std::string bookingDate = jsonString(row, "date");
        const std::string valuta = jsonString(row, "valuta");
        const double amount = jsonDouble(row, "amount");
        const std::string contractId = jsonString(row, "contractId");
        const std::string contractType = jsonString(row, "contractType");
        const auto propertyIds = jsonStringList(row, "propertyIds");
        const auto propertyNames = jsonStringList(row, "propertyNames");

        auto transaction = std::make_shared<core::domain::Transaction>(name, bookingDate, valuta, amount, jsonBool(row, "allocatable"));
        transaction->setId(id);
        transaction->setStatementId(jsonString(row, "statementId"));
        transaction->setContractId(contractId);
        transaction->setPropertyIds(propertyIds);
        transactions.push_back(std::move(transaction));

        if (!contractId.empty() && seenContracts.insert(contractId).second) {
            auto contract = std::make_shared<core::domain::Contract>();
            contract->setId(contractId);
            contract->rename(jsonString(row, "contractName"));
            contract->setType(contractType);
            contract->setPropertyIds(propertyIds);
            contracts.push_back(std::move(contract));
        }

        for (std::size_t p = 0; p < propertyIds.size(); ++p) {
            const auto& propertyId = propertyIds[p];
            if (propertyId.empty() || !seenProperties.insert(propertyId).second) {
                continue;
            }
            auto property = std::make_shared<core::domain::Property>();
            property->setId(propertyId);
            property->rename(p < propertyNames.size() && !propertyNames[p].empty() ? propertyNames[p] : propertyId);
            properties.push_back(std::move(property));
        }
    }

    catalog->setTransactions(std::move(transactions));
    catalog->setContracts(std::move(contracts));
    catalog->setProperties(std::move(properties));
    return catalog;
}

} // namespace

AnalysisResult AnalysisService::runAnalysis(const core::domain::catalog::WorkspaceCatalog& state,
                                           const AnalysisRequest& request) const
{
    return runAnalysisById(state, request.analysisId, request.filterSpecification);
}

AnalysisResult AnalysisService::runAnalysisById(const core::domain::catalog::WorkspaceCatalog& state,
                                               const std::string& analysisId,
                                               const std::string& filterSpec) const
{
    for (const auto& analysis : state.analyses()) {
        if (!analysis) {
            continue;
        }
        if (analysis->id() != analysisId) {
            continue;
        }

        const std::string effectiveFilter =
            filterSpec.empty() ? analysis->filterSpec() : filterSpec;
        return computeAnalysis(*analysis, state, effectiveFilter);
    }

    return AnalysisResult{};
}

AnalysisResult AnalysisService::computeAnalysis(const core::domain::Analysis& analysis,
                                               const core::domain::catalog::WorkspaceCatalog& state,
                                               const std::string& filterSpec) const
{
    AnalysisResult out;
    const auto frozenCatalog = catalogFromAnalysisSnapshot(analysis);
    const auto adjustedCatalog = analysis.includeCalculationAdjustments()
        ? catalogWithAdjustments(frozenCatalog ? *frozenCatalog : state, analysis.adjustments())
        : std::shared_ptr<core::domain::catalog::WorkspaceCatalog>{};
    const auto& sourceState = adjustedCatalog ? *adjustedCatalog : (frozenCatalog ? *frozenCatalog : state);
    const core::application::analysis::AnalysisFilter filter =
        core::application::analysis::parseAnalysisFilterSpec(filterSpec);

    const std::string key = analysis.executionType();

    if (key == core::constants::analysis::kTypePlot) {
        out = core::application::analysis::computePlotAnalysis(analysis, sourceState, filter);
    } else if (key == core::constants::analysis::kTypeTab) {
        out = core::application::analysis::computeTableAnalysis(analysis, sourceState, filter);
    } else if (key == core::constants::analysis::kTypeCalculation) {
        out = core::application::analysis::computeAdjustmentAnalysis(analysis, sourceState, filter);
    } else {
        out = core::application::analysis::computeTableAnalysis(analysis, sourceState, filter);
    }

    out.type = analysis.outputType();
    out.configJson = analysis.configJson();
    out.transactions = core::application::analysis::projectAnalysisTransactions(
        sourceState,
        core::application::analysis::collectAnalysisTransactions(sourceState, filter));
    out.generatedAt = analysis.updatedAt().empty()
        ? analysis.createdAt()
        : analysis.updatedAt();
    out.found = true;
    return out;
}

std::vector<std::shared_ptr<core::domain::Transaction>> AnalysisService::filterTransactions(
    const core::domain::catalog::WorkspaceCatalog& state,
    const std::string& filterSpec) const
{
    return collectAnalysisTransactions(state, parseAnalysisFilterSpec(filterSpec));
}

} // namespace core::application::analysis
