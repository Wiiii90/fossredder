/**
 * @file core/include/core/application/workspace/WorkspaceCatalogService.h
 * @brief Declares application services for mutating catalog entities in the app state.
 */

#pragma once

#include "core/domain/values/Alias.h"
#include "core/application/workspace/WorkspaceState.h"
#include "core/domain/entities/Transaction.h"

#include <string>
#include <vector>

namespace core::application {

struct ActorInput {
    std::string name;
    std::vector<core::domain::Alias> aliases;
};

struct PropertyInput {
    std::string name;
    std::vector<core::domain::Alias> aliases;
};

struct ContractInput {
    std::string name;
    std::string type;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    std::vector<core::domain::Alias> aliases;
};

struct TransactionInput {
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string statementId;
    core::domain::Transaction::Status status = core::domain::Transaction::Status::Neutral;
    std::string actorId;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
};

struct AnalysisInput {
    std::string name;
    std::string type;
    std::string configJson;
    std::string filterSpec;
    std::string exportFormat;
    bool includeCalcAdjustments = true;
    std::string exportStateJson;
    std::string snapshotTransactionsJson;
};

class WorkspaceCatalogService {
public:
    std::string addActor(core::domain::WorkspaceState& state, const ActorInput& input) const;
    bool updateActor(core::domain::WorkspaceState& state, const std::string& id, const ActorInput& input) const;
    bool deleteActor(core::domain::WorkspaceState& state, const std::string& id) const;

    std::string addProperty(core::domain::WorkspaceState& state, const PropertyInput& input) const;
    bool updateProperty(core::domain::WorkspaceState& state, const std::string& id, const PropertyInput& input) const;
    bool deleteProperty(core::domain::WorkspaceState& state, const std::string& id) const;

    std::string addContract(core::domain::WorkspaceState& state, const ContractInput& input) const;
    bool updateContract(core::domain::WorkspaceState& state, const std::string& id, const ContractInput& input) const;
    bool deleteContract(core::domain::WorkspaceState& state, const std::string& id) const;
    std::vector<std::string> contractTypes(const core::domain::WorkspaceState& state) const;

    std::string addStatement(core::domain::WorkspaceState& state, const std::string& name) const;
    bool updateStatement(core::domain::WorkspaceState& state, const std::string& id, const std::string& name) const;
    bool deleteStatement(core::domain::WorkspaceState& state, const std::string& id) const;

    std::string addTransaction(core::domain::WorkspaceState& state, const TransactionInput& input) const;
    bool updateTransaction(core::domain::WorkspaceState& state, const std::string& id, const TransactionInput& input) const;
    bool deleteTransaction(core::domain::WorkspaceState& state, const std::string& id) const;

    std::string addAnalysis(core::domain::WorkspaceState& state, const AnalysisInput& input) const;
    bool updateAnalysis(core::domain::WorkspaceState& state, const std::string& id, const AnalysisInput& input) const;
    bool deleteAnalysis(core::domain::WorkspaceState& state, const std::string& id) const;

    std::string addAnnual(core::domain::WorkspaceState& state,
                          const std::string& name,
                          int year,
                          const std::vector<std::string>& analysisIds = {}) const;
    bool updateAnnual(core::domain::WorkspaceState& state,
                      const std::string& id,
                      const std::string& name,
                      int year,
                      const std::vector<std::string>& analysisIds = {}) const;
    bool deleteAnnual(core::domain::WorkspaceState& state, const std::string& id) const;
};

using CatalogService = WorkspaceCatalogService;

}
