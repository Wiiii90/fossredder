/**
 * @file core/include/core/application/CatalogService.h
 * @brief Declares application services for mutating catalog entities in the app state.
 */

#pragma once

#include "core/models/AppState.h"
#include "core/models/Transaction.h"

#include <string>
#include <vector>

namespace core::application {

struct ActorInput {
    std::string name;
    std::string type;
    std::string description;
    std::vector<std::string> aliases;
};

struct PropertyInput {
    std::string name;
    std::string address;
    std::string description;
    std::vector<std::string> aliases;
};

struct ContractInput {
    std::string name;
    std::string type;
    std::string description;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    std::vector<std::string> aliases;
};

struct TransactionInput {
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string description;
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

class CatalogService {
public:
    std::string addActor(core::domain::AppState& state, const ActorInput& input) const;
    bool updateActor(core::domain::AppState& state, const std::string& id, const ActorInput& input) const;
    bool deleteActor(core::domain::AppState& state, const std::string& id) const;

    std::string addProperty(core::domain::AppState& state, const PropertyInput& input) const;
    bool updateProperty(core::domain::AppState& state, const std::string& id, const PropertyInput& input) const;
    bool deleteProperty(core::domain::AppState& state, const std::string& id) const;

    std::string addContract(core::domain::AppState& state, const ContractInput& input) const;
    bool updateContract(core::domain::AppState& state, const std::string& id, const ContractInput& input) const;
    bool deleteContract(core::domain::AppState& state, const std::string& id) const;
    std::vector<std::string> contractTypes(const core::domain::AppState& state) const;

    std::string addStatement(core::domain::AppState& state, const std::string& name) const;
    bool updateStatement(core::domain::AppState& state, const std::string& id, const std::string& name) const;
    bool deleteStatement(core::domain::AppState& state, const std::string& id) const;

    std::string addTransaction(core::domain::AppState& state, const TransactionInput& input) const;
    bool updateTransaction(core::domain::AppState& state, const std::string& id, const TransactionInput& input) const;
    bool deleteTransaction(core::domain::AppState& state, const std::string& id) const;

    std::string addAnalysis(core::domain::AppState& state, const AnalysisInput& input) const;
    bool updateAnalysis(core::domain::AppState& state, const std::string& id, const AnalysisInput& input) const;
    bool deleteAnalysis(core::domain::AppState& state, const std::string& id) const;

    std::string addAnnual(core::domain::AppState& state, int year) const;
    bool updateAnnual(core::domain::AppState& state, const std::string& id, int year) const;
    bool deleteAnnual(core::domain::AppState& state, const std::string& id) const;
};

}
